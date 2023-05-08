open Kore;

module T = AST.Type;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __component_id = "Foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context: AST.ParseContext.t(Language.Interface.program_t(AST.Type.t)) =
  AST.ParseContext.create(~report=ignore, __namespace);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | JSX"
  >::: [
    "resolve jsx with valid inline expression"
    >: (
      () => {
        let type_ = T.Valid(View([], Valid(Nil)));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.jsx(
          (
            __id |> URes.as_typed(type_),
            [],
            [],
            [
              "foo"
              |> URes.string_prim
              |> KSX.Child.of_inline
              |> URes.as_untyped,
            ],
          )
          |> KSX.of_component_tag,
          (
            URaw.as_untyped(__id),
            [],
            [],
            [
              "foo"
              |> URaw.string_prim
              |> KSX.Child.of_inline
              |> URaw.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope),
        );
      }
    ),
    "resolve jsx as tag if found in plugin scope"
    >: (
      () => {
        let type_ = T.Valid(View([], Valid(Nil)));
        let scope = {
          ...__throw_scope,
          context: {
            ...__throw_scope.context,
            modules: {
              ...__throw_scope.context.modules,
              plugins: [(ElementTag, [(Value, __id, type_)])],
            },
          },
        };

        Assert.jsx(
          (
            __id |> URes.as_typed(type_),
            [],
            [],
            [
              "foo"
              |> URes.string_prim
              |> KSX.Child.of_inline
              |> URes.as_untyped,
            ],
          )
          |> KSX.of_element_tag,
          (
            URaw.as_untyped(__id),
            [],
            [],
            [
              "foo"
              |> URaw.string_prim
              |> KSX.Child.of_inline
              |> URaw.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope),
        );
      }
    ),
    "resolve jsx with valid style binding"
    >: (
      () => {
        let style_id = "bar";
        let type_ = T.Valid(View([], Valid(Nil)));
        let scope = {
          ...__throw_scope,
          types:
            [(__id, type_), (style_id, T.Valid(Style))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.jsx(
          (
            __id |> URes.as_typed(type_),
            [style_id |> Expression.of_identifier |> URes.as_style],
            [],
            [],
          )
          |> KSX.of_component_tag,
          (
            URaw.as_untyped(__id),
            [style_id |> Expression.of_identifier |> URaw.as_untyped],
            [],
            [],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope),
        );
      }
    ),
    "report NotFound error with unrecognized tag identifier"
    >: (
      () => {
        Assert.throws_compile_errors(
          [ParseError(TypeError(NotFound(__id)), __namespace, Range.zero)],
          () =>
          (URaw.as_untyped(__id), [], [], [])
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, __throw_scope)
        );
      }
    ),
    "report InvalidKSXPrimitiveExpression error with invalid inline expression"
    >: (
      () => {
        let type_ = T.Valid(Function([], Valid(Boolean)));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidKSXPrimitiveExpression(type_)),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__id),
            [],
            [],
            [
              __id
              |> Expression.of_identifier
              |> URaw.as_node
              |> KSX.Child.of_inline
              |> URaw.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope)
        );
      }
    ),
    "report InvalidKSXPrimitiveExpression error with invalid view body"
    >: (
      () => {
        let type_ = T.Valid(Function([], Valid(Boolean)));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidKSXPrimitiveExpression(type_)),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          __id
          |> Expression.of_identifier
          |> URaw.as_node
          |> View.analyze_view_body(scope)
        );
      }
    ),
    "throw InvalidKSXTag error on jsx with incorrect type"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__component_id, T.Valid(Integer))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(
                InvalidKSXTag(
                  __component_id,
                  Valid(Integer),
                  [("bar", Valid(Boolean))],
                ),
              ),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__component_id),
            [],
            [
              (URaw.as_untyped("bar"), true |> URaw.bool_prim |> Option.some)
              |> URaw.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope)
        );
      }
    ),
    "throw InvalidKSXAttribute and UnexpectedKSXAttribute errors on jsx with incorrect attributes"
    >: (
      () => {
        let errors = ref([]);
        let scope = {
          ...__throw_scope,
          context: {
            ...__context,
            report: err => errors := errors^ @ [err],
          },
          types:
            [
              (
                __component_id,
                T.Valid(
                  View(
                    [("fizz", (T.Valid(Boolean), true))],
                    T.Valid(Element),
                  ),
                ),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        (
          URaw.as_untyped(__component_id),
          [],
          [
            (
              URaw.as_untyped("fizz"),
              "bar" |> URaw.string_prim |> Option.some,
            )
            |> URaw.as_untyped,
            (URaw.as_untyped("buzz"), true |> URaw.bool_prim |> Option.some)
            |> URaw.as_untyped,
          ],
          [],
        )
        |> KSX.of_element_tag
        |> KSX.analyze_ksx(Expression.analyze, scope)
        |> ignore;

        Assert.compile_errors(
          [
            ParseError(
              TypeError(UnexpectedKSXAttribute("buzz", Valid(Boolean))),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidKSXAttribute("fizz", Valid(Boolean), Valid(String)),
              ),
              __namespace,
              Range.zero,
            ),
          ],
          errors^,
        );
      }
    ),
    "resolve jsx with valid attributes"
    >: (
      () => {
        let errors = ref([]);
        let view_type =
          T.Valid(
            View(
              [
                ("fizz", (T.Valid(Boolean), true)),
                ("buzz", (T.Valid(String), true)),
                ("foobar", (T.Valid(Element), false)),
              ],
              T.Valid(Element),
            ),
          );
        let scope = {
          ...__throw_scope,
          context: {
            ...__context,
            report: err => errors := errors^ @ [err],
          },
          types:
            [(__component_id, view_type)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.jsx(
          (
            __component_id |> URes.as_typed(view_type),
            [],
            [
              (URes.as_untyped("fizz"), true |> URes.bool_prim |> Option.some)
              |> URes.as_untyped,
              (
                URes.as_untyped("buzz"),
                "bar" |> URes.string_prim |> Option.some,
              )
              |> URes.as_untyped,
            ],
            [],
          )
          |> KSX.of_component_tag,
          (
            URaw.as_untyped(__component_id),
            [],
            [
              (URaw.as_untyped("fizz"), true |> URaw.bool_prim |> Option.some)
              |> URaw.as_untyped,
              (
                URaw.as_untyped("buzz"),
                "bar" |> URaw.string_prim |> Option.some,
              )
              |> URaw.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> KSX.analyze_ksx(Expression.analyze, scope),
        );
      }
    ),
  ];
