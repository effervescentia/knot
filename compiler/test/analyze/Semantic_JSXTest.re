open Kore;

module Identifier = Reference.Identifier;
module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = Identifier.of_string("foo");
let __component_id = Identifier.of_string("Foo");
let __namespace = Reference.Namespace.of_string("foo");
let __throw_scope = S.create(__namespace, throw, Range.zero);

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
          |> SemanticAnalyzer.analyze_jsx(__throw_scope),
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
              |> URaw.as_unknown
              |> AR.of_inline_expr
              |> URaw.as_untyped,
            ],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(scope)
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
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_view_body(scope)
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
          |> SemanticAnalyzer.analyze_jsx(__throw_scope),
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
          |> SemanticAnalyzer.analyze_jsx(__throw_scope),
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
          |> SemanticAnalyzer.analyze_jsx(__throw_scope)
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
              (
                "bar" |> A.of_public |> URaw.as_untyped,
                true |> URaw.bool_prim |> Option.some,
              )
              |> AR.of_prop
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(scope)
        );
      }
    ),
    "throw InvalidJSXAttribute and UnexpectedJSXAttribute errors on jsx with incorrect attributes"
    >: (
      () => {
        let errors = ref([]);
        let scope = {
          ...__throw_scope,
          report: err => errors := errors^ @ [err],
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
              "fizz" |> A.of_public |> URaw.as_untyped,
              "bar" |> URaw.string_prim |> Option.some,
            )
            |> AR.of_prop
            |> URaw.as_untyped,
            (
              "buzz" |> A.of_public |> URaw.as_untyped,
              true |> URaw.bool_prim |> Option.some,
            )
            |> AR.of_prop
            |> URaw.as_untyped,
          ],
          [],
        )
        |> AR.of_tag
        |> SemanticAnalyzer.analyze_jsx(scope)
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
          report: err => errors := errors^ @ [err],
          types:
            [(__component_id, view_type)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.jsx(
          (
            __component_id |> URes.as_typed(view_type),
            [
              (
                "fizz" |> A.of_public |> URes.as_untyped,
                true |> URes.bool_prim |> Option.some,
              )
              |> A.of_prop
              |> URes.as_untyped,
              (
                "buzz" |> A.of_public |> URes.as_untyped,
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
              (
                "fizz" |> A.of_public |> URaw.as_untyped,
                true |> URaw.bool_prim |> Option.some,
              )
              |> AR.of_prop
              |> URaw.as_untyped,
              (
                "buzz" |> A.of_public |> URaw.as_untyped,
                "bar" |> URaw.string_prim |> Option.some,
              )
              |> AR.of_prop
              |> URaw.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(scope),
        );
      }
    ),
  ];
