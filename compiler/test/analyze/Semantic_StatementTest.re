open Kore;

module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = ParseContext.create(~report=ignore, __namespace);
let __scope = S.create(__context, Range.zero);
let __throw_scope = S.create({...__context, report: throw}, Range.zero);

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
          |> KStatement.Plugin.analyze(__scope, KExpression.Plugin.analyze),
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
          |> KStatement.Plugin.analyze(__scope, KExpression.Plugin.analyze),
        )
    ),
  ];
