open Knot.Kore;
open Parse.Onyx;

module Keyword = Parse.Keyword;

let declaration = (ctx: ParseContext.t) =>
  AST.of_main_export
  <$ Keyword.main
  |> option(AST.of_named_export)
  >>= (
    f =>
      choice([
        KConstant.Plugin.parse(ctx, f),
        KEnumerated.Plugin.parse(ctx, f),
        KFunction.Plugin.parse(ctx, f),
        KView.Plugin.parse(ctx, f),
      ])
      >|= Node.map(AST.of_decl)
  );
