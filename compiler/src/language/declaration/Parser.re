open Knot.Kore;
open Parse.Kore;
open AST;

let declaration = (ctx: ParseContext.t) =>
  Result.of_main_export
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(Result.of_named_export)
  >|= Tuple.with_fst2(ctx)
  >>= (
    arg =>
      choice([
        KConstant.Plugin.parse(arg),
        KEnumerated.Plugin.parse(arg),
        KFunction.Plugin.parse(arg),
        KView.Plugin.parse(arg),
      ])
      >|= Node.map(Result.of_decl)
  );
