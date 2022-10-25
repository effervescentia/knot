open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;

let declaration = (ctx: ParseContext.t) =>
  AST.of_main_export
  <$ Keyword.main
  |> option(AST.of_named_export)
  >>= (
    f =>
      choice([
        KConstant.Plugin.parse(ctx, f),
        KEnumerated.Plugin.parse(ctx, f),
        KExpression.Plugin.parse |> KFunction.Plugin.parse(ctx, f),
        KExpression.Plugin.parse |> KView.Plugin.parse(ctx, f),
      ])
      >|= Node.map(AST.of_decl)
  );
