open Kore;

module Identifier = Reference.Identifier;
module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = Identifier.of_string("foo");
let __namespace = Reference.Namespace.of_string("foo");
let __scope = S.create(__namespace, ignore, Range.zero);
let __throw_scope = S.create(__namespace, throw, Range.zero);

let suite =
  "Analyze.Semantic | Statement"
  >::: [
    "extract expression type"
    >: (
      () =>
        Assert.statement(
          "foo" |> URes.string_prim |> A.of_expr |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_expr
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_statement(__scope),
        )
    ),
    "resolve nil for variable declaration"
    >: (
      () =>
        Assert.statement(
          "foo" |> URes.string_prim |> A.of_expr |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_expr
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_statement(__scope),
        )
    ),
  ];
