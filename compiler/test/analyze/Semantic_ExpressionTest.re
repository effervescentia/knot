open Kore;

module A = AST.Result;
module AR = AST.Raw;
module T = AST.Type;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = AST.ParseContext.create(~report=ignore, __namespace);
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
          URes.bool_prim(true),
          true |> URaw.bool_prim |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "extract group inner expression type"
    >: (
      () =>
        Assert.expression(
          "foo" |> URes.string_prim |> A.of_group |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_group
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "extract last closure statement type"
    >: (
      () =>
        Assert.expression(
          [123 |> URes.int_prim |> A.of_effect |> URes.as_int]
          |> A.of_closure
          |> URes.as_int,
          [123 |> URaw.int_prim |> AR.of_effect |> URaw.as_node]
          |> AR.of_closure
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "resolve nil if closure empty"
    >: (
      () =>
        Assert.expression(
          [] |> A.of_closure |> URes.as_nil,
          []
          |> AR.of_closure
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "always resolve JSX as element type"
    >: (
      () => {
        let type_ = T.Valid(`View(([], Valid(`Nil))));
        let scope = {
          ...__scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          (__id |> URes.as_typed(type_), [], [], [])
          |> A.of_component_tag
          |> A.of_ksx
          |> URes.as_element,
          (URaw.as_untyped(__id), [], [], [])
          |> AR.of_element_tag
          |> AR.of_ksx
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve NotInferrable type on unrecognized identifier"
    >: (
      () =>
        Assert.expression(
          __id |> A.of_id |> URes.as_invalid(NotInferrable),
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__scope),
        )
    ),
    "report NotFound exception on unrecognized identifier"
    >: (
      () =>
        Assert.throws_compile_errors(
          [ParseError(TypeError(NotFound(__id)), __namespace, Range.zero)],
          () =>
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(__throw_scope)
        )
    ),
    "resolve recognized identifier"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__id, T.Valid(`Boolean))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          __id |> A.of_id |> URes.as_bool,
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve dot access"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [
              (
                __id,
                T.Valid(`Object([("foo", (Valid(`Boolean), true))])),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            __id
            |> A.of_id
            |> URes.as_struct([("foo", (T.Valid(`Boolean), true))]),
            URes.as_untyped("foo"),
          )
          |> A.of_dot_access
          |> URes.as_bool,
          (__id |> AR.of_id |> URaw.as_node, URaw.as_untyped("foo"))
          |> AR.of_dot_access
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
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
              (view_id, T.Valid(`View(([], Valid(`Element))))),
              (style_id, T.Valid(`Style)),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            view_id |> A.of_id |> URes.as_view([], Valid(`Element)),
            style_id |> A.of_id |> URes.as_style,
          )
          |> A.of_component_bind_style
          |> URes.as_view([], Valid(`Element)),
          (
            view_id |> AR.of_id |> URaw.as_node,
            style_id |> AR.of_id |> URaw.as_node,
          )
          |> AR.of_component_bind_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
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
                      "width",
                      Value(
                        Valid(`Function(([Valid(`String)], Valid(`Nil)))),
                      ),
                    ),
                  ],
                ),
                (
                  Reference.Plugin.StyleExpression,
                  [
                    (
                      "px",
                      Value(
                        Valid(
                          `Function(([Valid(`Integer)], Valid(`String))),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            },
          },
          types:
            [(view_id, T.Valid(`View(([], Valid(`Element)))))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            view_id |> A.of_id |> URes.as_view([], Valid(`Element)),
            [
              (
                "width" |> URes.as_string,
                (
                  "$px"
                  |> A.of_id
                  |> URes.as_function([Valid(`Integer)], Valid(`String)),
                  [URes.int_prim(12)],
                )
                |> A.of_func_call
                |> URes.as_string,
              )
              |> URes.as_untyped,
            ]
            |> A.of_style
            |> URes.as_style,
          )
          |> A.of_component_bind_style
          |> URes.as_view([], Valid(`Element)),
          (
            view_id |> AR.of_id |> URaw.as_node,
            [
              (
                "width" |> URaw.as_untyped,
                ("$px" |> AR.of_id |> URaw.as_untyped, [URaw.int_prim(12)])
                |> AR.of_func_call
                |> URaw.as_untyped,
              )
              |> URaw.as_untyped,
            ]
            |> AR.of_style
            |> URaw.as_node,
          )
          |> AR.of_component_bind_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
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
                  [(tag_id, Value(Valid(`View(([], Valid(`Element))))))],
                ),
              ],
            },
          },
          types:
            [(style_id, T.Valid(`Style))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            tag_id |> A.of_id |> URes.as_view([], Valid(`Element)),
            style_id |> A.of_id |> URes.as_style,
          )
          |> A.of_element_bind_style
          |> URes.as_view([], Valid(`Element)),
          (
            tag_id |> AR.of_id |> URaw.as_node,
            style_id |> AR.of_id |> URaw.as_node,
          )
          |> AR.of_element_bind_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
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
                  [(tag_id, Value(Valid(`View(([], Valid(`Element))))))],
                ),
                (
                  Reference.Plugin.StyleRule,
                  [
                    (
                      "width",
                      Value(
                        Valid(`Function(([Valid(`String)], Valid(`Nil)))),
                      ),
                    ),
                  ],
                ),
                (
                  Reference.Plugin.StyleExpression,
                  [
                    (
                      "px",
                      Value(
                        Valid(
                          `Function(([Valid(`Integer)], Valid(`String))),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            },
          },
        };

        Assert.expression(
          (
            tag_id |> A.of_id |> URes.as_view([], Valid(`Element)),
            [
              (
                "width" |> URes.as_string,
                (
                  "$px"
                  |> A.of_id
                  |> URes.as_function([Valid(`Integer)], Valid(`String)),
                  [URes.int_prim(12)],
                )
                |> A.of_func_call
                |> URes.as_string,
              )
              |> URes.as_untyped,
            ]
            |> A.of_style
            |> URes.as_style,
          )
          |> A.of_element_bind_style
          |> URes.as_view([], Valid(`Element)),
          (
            tag_id |> AR.of_id |> URaw.as_node,
            [
              (
                "width" |> URaw.as_untyped,
                ("$px" |> AR.of_id |> URaw.as_untyped, [URaw.int_prim(12)])
                |> AR.of_func_call
                |> URaw.as_untyped,
              )
              |> URaw.as_untyped,
            ]
            |> AR.of_style
            |> URaw.as_node,
          )
          |> AR.of_element_bind_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve function call"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [
              (
                __id,
                T.Valid(`Function(([Valid(`Integer)], Valid(`String)))),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.expression(
          (
            __id
            |> A.of_id
            |> URes.as_function([T.Valid(`Integer)], T.Valid(`String)),
            [URes.int_prim(123)],
          )
          |> A.of_func_call
          |> URes.as_string,
          (__id |> AR.of_id |> URaw.as_node, [URaw.int_prim(123)])
          |> AR.of_func_call
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
    "resolve style expression"
    >: (
      () => {
        let fizz_type =
          T.Valid(`Function(([Valid(`Integer)], Valid(`Boolean))));
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
                    ("fizz", Value(fizz_type)),
                    ("buzz", Value(Valid(`String))),
                  ],
                ),
                (
                  StyleRule,
                  [
                    (
                      "foo",
                      Value(
                        Valid(
                          `Function(([Valid(`Boolean)], Valid(`Nil))),
                        ),
                      ),
                    ),
                    (
                      "bar",
                      Value(
                        Valid(
                          `Function(([Valid(`Integer)], Valid(`Nil))),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            },
          },
        };

        Assert.expression(
          [
            (
              "foo" |> URes.as_bool,
              (
                "$fizz" |> A.of_id |> URes.as_typed(fizz_type),
                [URes.int_prim(123)],
              )
              |> A.of_func_call
              |> URes.as_bool,
            )
            |> URes.as_untyped,
            ("bar" |> URes.as_int, "$buzz" |> A.of_id |> URes.as_string)
            |> URes.as_untyped,
          ]
          |> A.of_style
          |> URes.as_style,
          [
            (
              "foo" |> URaw.as_node,
              ("$fizz" |> AR.of_id |> URaw.as_node, [URaw.int_prim(123)])
              |> AR.of_func_call
              |> URaw.as_node,
            )
            |> URes.as_untyped,
            ("bar" |> URaw.as_node, "$buzz" |> AR.of_id |> URaw.as_node)
            |> URes.as_untyped,
          ]
          |> AR.of_style
          |> URaw.as_node
          |> KExpression.Plugin.analyze(scope),
        );
      }
    ),
  ];
