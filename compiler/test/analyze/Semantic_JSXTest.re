open Kore;

module Identifier = Reference.Identifier;
module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtilV2;
module URes = Util.ResultUtilV2;

let __id = Identifier.of_string("foo");
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
            URes.as_raw_node(__id),
            [],
            [
              "foo" |> URes.string_prim |> A.of_inline_expr |> URes.as_raw_node,
            ],
          )
          |> A.of_tag,
          (
            URaw.as_raw_node(__id),
            [],
            [
              "foo" |> URaw.string_prim |> AR.of_inline_expr |> URaw.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(__throw_scope),
        )
    ),
    "report InvalidJSXInlineExpression error with invalid inline expression"
    >: (
      () => {
        let type_ = T.Valid(`Function(([], Valid(`Boolean))));
        let scope = {
          ...__throw_scope,
          types: [(__id, type_)] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.throws(
          CompileError([
            ParseError(
              TypeErrorV2(InvalidJSXInlineExpression(type_)),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw InvalidJSXInlineExpression",
          () =>
          (
            URaw.as_raw_node(__id),
            [],
            [
              __id
              |> AR.of_id
              |> URaw.as_unknown
              |> AR.of_inline_expr
              |> URaw.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(scope)
        );
      }
    ),
    "resolve jsx with empty class expression"
    >: (
      () =>
        Assert.jsx(
          (
            URes.as_raw_node(__id),
            [
              (URes.as_raw_node(__id), None)
              |> A.of_jsx_class
              |> URes.as_raw_node,
            ],
            [],
          )
          |> A.of_tag,
          (
            URaw.as_raw_node(__id),
            [
              (URaw.as_raw_node(__id), None)
              |> AR.of_jsx_class
              |> URaw.as_raw_node,
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
            URes.as_raw_node(__id),
            [
              (URes.as_raw_node(__id), Some(URes.bool_prim(true)))
              |> A.of_jsx_class
              |> URes.as_raw_node,
            ],
            [],
          )
          |> A.of_tag,
          (
            URaw.as_raw_node(__id),
            [
              (URaw.as_raw_node(__id), Some(URaw.bool_prim(true)))
              |> AR.of_jsx_class
              |> URaw.as_raw_node,
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
        Assert.throws(
          CompileError([
            ParseError(
              TypeErrorV2(InvalidJSXClassExpression(Valid(`String))),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw InvalidJSXClassExpression",
          () =>
          (
            URaw.as_raw_node(__id),
            [
              (URaw.as_raw_node(__id), Some(URaw.string_prim("foo")))
              |> AR.of_jsx_class
              |> URaw.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> SemanticAnalyzer.analyze_jsx(__throw_scope)
        )
    ),
  ];
