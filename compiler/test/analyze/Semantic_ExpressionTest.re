open Kore;

module T = AST.Type;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context: AST.ParseContext.t(Language.Interface.program_t(AST.Type.t)) =
  AST.ParseContext.create(~report=ignore, __namespace);
let __scope = AST.Scope.create(__context, Range.zero);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | Expression"
  >::: [
    "adopt primitive type"
    >: (
      () =>
        Assert.expression(
          (
            true |> Primitive.of_boolean |> Expression.of_primitive,
            Valid(Boolean),
          ),
          true |> URaw.bool_prim |> Expression.analyze(__scope),
        )
    ),
    "extract group inner expression type"
    >: (
      () =>
        Assert.expression(
          (
            "foo" |> URes.string_prim |> Expression.of_group,
            AST.Type.Valid(String),
          ),
          "foo"
          |> URaw.string_prim
          |> Expression.of_group
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "extract last closure statement type"
    >: (
      () =>
        Assert.expression(
          (
            [123 |> URes.int_prim |> Statement.of_effect |> URes.as_int]
            |> Expression.of_closure,
            Valid(Integer),
          ),
          [123 |> URaw.int_prim |> Statement.of_effect |> URaw.as_node]
          |> Expression.of_closure
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "resolve nil if closure empty"
    >: (
      () =>
        Assert.expression(
          ([] |> Expression.of_closure, Valid(Nil)),
          []
          |> Expression.of_closure
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "always resolve JSX as element type"
    >: (
      () => {
        let type_ = T.Valid(View([], Valid(Nil)));
        let scope = {
          ...__scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (__id |> URes.as_typed(type_), [], [], [])
            |> KSX.of_component_tag
            |> Expression.of_ksx,
            Valid(Element),
          ),
          (URaw.as_untyped(__id), [], [], [])
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve NotInferrable type on unrecognized identifier"
    >: (
      () =>
        Assert.expression(
          (__id |> Expression.of_identifier, Invalid(NotInferrable)),
          __id
          |> Expression.of_identifier
          |> URaw.as_node
          |> Expression.analyze(__scope),
        )
    ),
    "report NotFound exception on unrecognized identifier"
    >: (
      () =>
        Assert.throws_compile_errors(
          [ParseError(TypeError(NotFound(__id)), __namespace, Range.zero)],
          () =>
          __id
          |> Expression.of_identifier
          |> URaw.as_node
          |> Expression.analyze(__throw_scope)
        )
    ),
    "resolve recognized identifier"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(Boolean))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          (__id |> Expression.of_identifier, Valid(Boolean)),
          __id
          |> Expression.of_identifier
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve dot access"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(Object([("foo", (Valid(Boolean), true))])))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (
              __id
              |> Expression.of_identifier
              |> URes.as_struct([("foo", (T.Valid(Boolean), true))]),
              URes.as_untyped("foo"),
            )
            |> Expression.of_dot_access,
            Valid(Boolean),
          ),
          (
            __id |> Expression.of_identifier |> URaw.as_node,
            URaw.as_untyped("foo"),
          )
          |> Expression.of_dot_access
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve style binding"
    >: (
      () => {
        let view_id = "my_view";
        let style_id = "my_style";
        let scope = {
          ...__throw_scope,
          types:
            [
              (view_id, T.Valid(View([], Valid(Element)))),
              (style_id, T.Valid(Style)),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (
              view_id
              |> Expression.of_identifier
              |> URes.as_view([], Valid(Element)),
              style_id |> Expression.of_identifier |> URes.as_style,
            )
            |> Expression.of_bind_component_style,
            Valid(View([], Valid(Element))),
          ),
          (
            view_id |> Expression.of_identifier |> URaw.as_node,
            style_id |> Expression.of_identifier |> URaw.as_node,
          )
          |> Expression.of_bind_component_style
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve style literal binding"
    >: (
      () => {
        let view_id = "my_view";
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [
                (
                  Reference.Plugin.StyleRule,
                  [
                    (
                      Value,
                      "width",
                      Valid(Function([Valid(String)], Valid(Nil))),
                    ),
                  ],
                ),
                (
                  Reference.Plugin.StyleExpression,
                  [
                    (
                      Value,
                      "px",
                      Valid(Function([Valid(Integer)], Valid(String))),
                    ),
                  ],
                ),
              ],
            },
          },
          types:
            [(view_id, T.Valid(View([], Valid(Element))))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (
              view_id
              |> Expression.of_identifier
              |> URes.as_view([], Valid(Element)),
              [
                (
                  "width" |> URes.as_string,
                  (
                    "$px"
                    |> Expression.of_identifier
                    |> URes.as_function([Valid(Integer)], Valid(String)),
                    [URes.int_prim(12)],
                  )
                  |> Expression.of_function_call
                  |> URes.as_string,
                )
                |> URes.as_untyped,
              ]
              |> Expression.of_style
              |> URes.as_style,
            )
            |> Expression.of_bind_component_style,
            Valid(View([], Valid(Element))),
          ),
          (
            view_id |> Expression.of_identifier |> URaw.as_node,
            [
              (
                "width" |> URaw.as_untyped,
                (
                  "$px" |> Expression.of_identifier |> URaw.as_untyped,
                  [URaw.int_prim(12)],
                )
                |> Expression.of_function_call
                |> URaw.as_untyped,
              )
              |> URaw.as_untyped,
            ]
            |> Expression.of_style
            |> URaw.as_node,
          )
          |> Expression.of_bind_component_style
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve built-in tag style binding"
    >: (
      () => {
        let tag_id = "tag";
        let style_id = "my_style";
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [
                (
                  Reference.Plugin.ElementTag,
                  [(Value, tag_id, Valid(View([], Valid(Element))))],
                ),
              ],
            },
          },
          types:
            [(style_id, T.Valid(Style))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (
              tag_id
              |> Expression.of_identifier
              |> URes.as_view([], Valid(Element)),
              style_id |> Expression.of_identifier |> URes.as_style,
            )
            |> Expression.of_bind_element_style,
            Valid(View([], Valid(Element))),
          ),
          (
            tag_id |> Expression.of_identifier |> URaw.as_node,
            style_id |> Expression.of_identifier |> URaw.as_node,
          )
          |> Expression.of_bind_element_style
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve built-in tag style literal binding"
    >: (
      () => {
        let tag_id = "tag";
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [
                (
                  Reference.Plugin.ElementTag,
                  [(Value, tag_id, Valid(View([], Valid(Element))))],
                ),
                (
                  Reference.Plugin.StyleRule,
                  [
                    (
                      Value,
                      "width",
                      Valid(Function([Valid(String)], Valid(Nil))),
                    ),
                  ],
                ),
                (
                  Reference.Plugin.StyleExpression,
                  [
                    (
                      Value,
                      "px",
                      Valid(Function([Valid(Integer)], Valid(String))),
                    ),
                  ],
                ),
              ],
            },
          },
        };

        Assert.expression(
          (
            (
              tag_id
              |> Expression.of_identifier
              |> URes.as_view([], Valid(Element)),
              [
                (
                  "width" |> URes.as_string,
                  (
                    "$px"
                    |> Expression.of_identifier
                    |> URes.as_function([Valid(Integer)], Valid(String)),
                    [URes.int_prim(12)],
                  )
                  |> Expression.of_function_call
                  |> URes.as_string,
                )
                |> URes.as_untyped,
              ]
              |> Expression.of_style
              |> URes.as_style,
            )
            |> Expression.of_bind_element_style,
            Valid(View([], Valid(Element))),
          ),
          (
            tag_id |> Expression.of_identifier |> URaw.as_node,
            [
              (
                "width" |> URaw.as_untyped,
                (
                  "$px" |> Expression.of_identifier |> URaw.as_untyped,
                  [URaw.int_prim(12)],
                )
                |> Expression.of_function_call
                |> URaw.as_untyped,
              )
              |> URaw.as_untyped,
            ]
            |> Expression.of_style
            |> URaw.as_node,
          )
          |> Expression.of_bind_element_style
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve function call"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(Function([Valid(Integer)], Valid(String))))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            (
              __id
              |> Expression.of_identifier
              |> URes.as_function([T.Valid(Integer)], T.Valid(String)),
              [URes.int_prim(123)],
            )
            |> Expression.of_function_call,
            Valid(String),
          ),
          (
            __id |> Expression.of_identifier |> URaw.as_node,
            [URaw.int_prim(123)],
          )
          |> Expression.of_function_call
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
    "resolve style expression"
    >: (
      () => {
        let fizz_type =
          T.Valid(Function([Valid(Integer)], Valid(Boolean)));
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [
                (
                  StyleExpression,
                  [
                    (Value, "fizz", fizz_type),
                    (Value, "buzz", Valid(String)),
                  ],
                ),
                (
                  StyleRule,
                  [
                    (
                      Value,
                      "foo",
                      Valid(Function([Valid(Boolean)], Valid(Nil))),
                    ),
                    (
                      Value,
                      "bar",
                      Valid(Function([Valid(Integer)], Valid(Nil))),
                    ),
                  ],
                ),
              ],
            },
          },
        };

        Assert.expression(
          (
            [
              (
                "foo" |> URes.as_bool,
                (
                  "$fizz"
                  |> Expression.of_identifier
                  |> URes.as_typed(fizz_type),
                  [URes.int_prim(123)],
                )
                |> Expression.of_function_call
                |> URes.as_bool,
              )
              |> URes.as_untyped,
              (
                "bar" |> URes.as_int,
                "$buzz" |> Expression.of_identifier |> URes.as_string,
              )
              |> URes.as_untyped,
            ]
            |> Expression.of_style,
            Valid(Style),
          ),
          [
            (
              "foo" |> URaw.as_node,
              (
                "$fizz" |> Expression.of_identifier |> URaw.as_node,
                [URaw.int_prim(123)],
              )
              |> Expression.of_function_call
              |> URaw.as_node,
            )
            |> URes.as_untyped,
            (
              "bar" |> URaw.as_node,
              "$buzz" |> Expression.of_identifier |> URaw.as_node,
            )
            |> URes.as_untyped,
          ]
          |> Expression.of_style
          |> URaw.as_node
          |> Expression.analyze(scope),
        );
      }
    ),
  ];
