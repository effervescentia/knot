open Kore;
open Parse.Kore;
open AST;

let parse = (ctx: ParseContext.t) =>
  Result.of_main_export
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(Result.of_named_export)
  >|= Tuple.with_fst2(ctx)
  >>= (
    arg =>
      choice([
        KConstant.parse(arg),
        KEnumerated.parse(arg),
        KFunction.parse(arg),
        KView.parse(arg),
      ])
      >|= Node.map(Result.of_decl)
  );
