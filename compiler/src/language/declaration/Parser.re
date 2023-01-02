open Kore;
open Parse.Kore;
open AST;

let parse = (ctx: ParseContext.t) =>
  Module.Main
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(Module.Named)
  >|= Tuple.with_fst2(ctx)
  >>= (
    ((_, export) as arg) =>
      choice([
        KConstant.parse(arg),
        KEnumerated.parse(arg),
        KFunction.parse(arg),
        KView.parse(arg),
      ])
      >|= Node.map(((name, declaration)) =>
            (export, name, declaration) |> Result.of_export
          )
  );
