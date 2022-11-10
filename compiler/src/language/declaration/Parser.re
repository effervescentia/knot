open Knot.Kore;
open Parse.Kore;
open AST;

let declaration = (ctx: ParseContext.t) =>
  Result.of_main_export
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(Result.of_named_export)
  >>= (
    f =>
      choice([
        KConstant.Plugin.parse(ctx, f),
        KEnumerated.Plugin.parse(ctx, f),
        KFunction.Plugin.parse(ctx, f),
        KView.Plugin.parse(ctx, f),
      ])
      >|= Node.map(Result.of_decl)
  );
