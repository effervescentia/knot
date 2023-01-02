open Kore;

module A = AST.Result;
module AR = AST.Raw;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = AST.ParseContext.create(~report=ignore, __namespace);
let __scope = AST.Scope.create(__context, Range.zero);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | Statement"
  >::: [
    "extract expression type"
    >: (
      () =>
        Assert.statement(
          "foo" |> URes.string_prim |> A.of_effect |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_effect
          |> URaw.as_node
          |> KStatement.Plugin.analyze(__scope, KExpression.Plugin.analyze),
        )
    ),
    "resolve nil for variable declaration"
    >: (
      () =>
        Assert.statement(
          "foo" |> URes.string_prim |> A.of_effect |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_effect
          |> URaw.as_node
          |> KStatement.Plugin.analyze(__scope, KExpression.Plugin.analyze),
        )
    ),
  ];
