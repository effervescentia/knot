open Kore;

module A = AST.Result;
module AR = AST.Raw;
module T = AST.Type;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __component_id = "Foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = AST.ParseContext.create(~report=ignore, __namespace);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | JSX"
  >::: [
    "resolve jsx with valid inline expression"
    >: (
      () =>
        Assert.jsx(
          (
            URes.as_untyped(__id),
            [],
            [
              "foo" |> URes.string_prim |> A.of_inline_expr |> URes.as_untyped,
            ],
          )
          |> A.of_tag,
          (
            URaw.as_untyped(__id),
            [],
            [
              "foo" |> URaw.string_prim |> AR.of_inline_expr |> URaw.as_untyped,
            ],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(
               __throw_scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "report InvalidJSXPrimitiveExpression error with invalid inline expression"
    >: (
      () => {
        let type_ = T.Valid(`Function(([], Valid(`Boolean))));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidJSXPrimitiveExpression(type_)),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__id),
            [],
            [
              __id
              |> AR.of_id
              |> URaw.as_node
              |> AR.of_inline_expr
              |> URaw.as_untyped,
            ],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(scope, KExpression.Plugin.analyze)
        );
      }
    ),
    "report InvalidJSXPrimitiveExpression error with invalid view body"
    >: (
      () => {
        let type_ = T.Valid(`Function(([], Valid(`Boolean))));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidJSXPrimitiveExpression(type_)),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          __id
          |> AR.of_id
          |> URaw.as_node
          |> KView.Analyzer.analyze_view_body(
               scope,
               KExpression.Plugin.analyze,
             )
        );
      }
    ),
    "resolve jsx with empty class expression"
    >: (
      () =>
        Assert.jsx(
          (
            URes.as_untyped(__id),
            [
              (URes.as_untyped(__id), None)
              |> A.of_jsx_class
              |> URes.as_untyped,
            ],
            [],
          )
          |> A.of_tag,
          (
            URaw.as_untyped(__id),
            [
              (URaw.as_untyped(__id), None)
              |> AR.of_jsx_class
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(
               __throw_scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "resolve jsx with valid class expression"
    >: (
      () =>
        Assert.jsx(
          (
            URes.as_untyped(__id),
            [
              (URes.as_untyped(__id), Some(URes.bool_prim(true)))
              |> A.of_jsx_class
              |> URes.as_untyped,
            ],
            [],
          )
          |> A.of_tag,
          (
            URaw.as_untyped(__id),
            [
              (URaw.as_untyped(__id), Some(URaw.bool_prim(true)))
              |> AR.of_jsx_class
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(
               __throw_scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "throw InvalidJSXClassExpression error on jsx with invalid class expression"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(InvalidJSXClassExpression(Valid(`String))),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__id),
            [
              (URaw.as_untyped(__id), Some(URaw.string_prim("foo")))
              |> AR.of_jsx_class
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(
               __throw_scope,
               KExpression.Plugin.analyze,
             )
        )
    ),
    "throw InvalidJSXTag error on jsx with incorrect type"
    >: (
      () => {
        let scope = {
          ...__throw_scope,
          types:
            [(__component_id, T.Valid(`Integer))]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(
                InvalidJSXTag(
                  __component_id,
                  Valid(`Integer),
                  [("bar", Valid(`Boolean))],
                ),
              ),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__component_id),
            [
              (URaw.as_untyped("bar"), true |> URaw.bool_prim |> Option.some)
              |> AR.of_prop
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(scope, KExpression.Plugin.analyze)
        );
      }
    ),
    "throw InvalidJSXAttribute and UnexpectedJSXAttribute errors on jsx with incorrect attributes"
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
                  `View((
                    [("fizz", T.Valid(`Boolean))],
                    T.Valid(`Element),
                  )),
                ),
              ),
            ]
            |> List.to_seq
            |> Hashtbl.of_seq,
        };

        (
          URaw.as_untyped(__component_id),
          [
            (
              URaw.as_untyped("fizz"),
              "bar" |> URaw.string_prim |> Option.some,
            )
            |> AR.of_prop
            |> URaw.as_untyped,
            (URaw.as_untyped("buzz"), true |> URaw.bool_prim |> Option.some)
            |> AR.of_prop
            |> URaw.as_untyped,
          ],
          [],
        )
        |> AR.of_tag
        |> KSX.Analyzer.analyze_jsx(scope, KExpression.Plugin.analyze)
        |> ignore;

        Assert.compile_errors(
          [
            ParseError(
              TypeError(UnexpectedJSXAttribute("buzz", Valid(`Boolean))),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidJSXAttribute(
                  "fizz",
                  Valid(`Boolean),
                  Valid(`String),
                ),
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
            `View((
              [("fizz", T.Valid(`Boolean)), ("buzz", T.Valid(`String))],
              T.Valid(`Element),
            )),
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
            [
              (URes.as_untyped("fizz"), true |> URes.bool_prim |> Option.some)
              |> A.of_prop
              |> URes.as_untyped,
              (
                URes.as_untyped("buzz"),
                "bar" |> URes.string_prim |> Option.some,
              )
              |> A.of_prop
              |> URes.as_untyped,
            ],
            [],
          )
          |> A.of_component,
          (
            URaw.as_untyped(__component_id),
            [
              (URaw.as_untyped("fizz"), true |> URaw.bool_prim |> Option.some)
              |> AR.of_prop
              |> URaw.as_untyped,
              (
                URaw.as_untyped("buzz"),
                "bar" |> URaw.string_prim |> Option.some,
              )
              |> AR.of_prop
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> KSX.Analyzer.analyze_jsx(scope, KExpression.Plugin.analyze),
        );
      }
    ),
  ];
