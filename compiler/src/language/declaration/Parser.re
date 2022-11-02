open Knot.Kore;
open Parse.Onyx;

module Keyword = Parse.Keyword;
module ParseContext = AST.ParseContext;

let declaration = (ctx: ParseContext.t) =>
  AST.Result.of_main_export
  <$ Keyword.main
  |> option(AST.Result.of_named_export)
  >>= (
    f =>
      choice([
        KConstant.Plugin.parse(ctx, f),
        KEnumerated.Plugin.parse(ctx, f),
        KFunction.Plugin.parse(ctx, f),
        KView.Plugin.parse(ctx, f),
      ])
      >|= Node.map(AST.Result.of_decl)
  );
