open Kore;

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
  "Analyze.Semantic | Statement"
  >::: [
    "extract expression type"
    >: (
      () =>
        Assert.statement(
          ("foo" |> URes.string_prim |> Statement.of_effect, Valid(String)),
          "foo"
          |> URaw.string_prim
          |> Statement.of_effect
          |> URaw.as_node
          |> Statement.analyze(Expression.analyze, __scope),
        )
    ),
    "resolve nil for variable declaration"
    >: (
      () =>
        Assert.statement(
          ("foo" |> URes.string_prim |> Statement.of_effect, Valid(String)),
          "foo"
          |> URaw.string_prim
          |> Statement.of_effect
          |> URaw.as_node
          |> Statement.analyze(Expression.analyze, __scope),
        )
    ),
  ];
