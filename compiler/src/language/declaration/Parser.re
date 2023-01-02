open Kore;
open Parse.Kore;
open AST;

let parse = (ctx: ParseContext.t) =>
  Module.Main
  <$ Matchers.keyword(Constants.Keyword.main)
  |> option(Module.Named)
  >|= Tuple.with_fst2(ctx)
  >>= (
    ((_, export) as arg) => {
      let bind = (parse, tag) =>
        parse(arg) >|= Node.map(Tuple.map_snd2(Node.map(tag)));

      choice([
        bind(KConstant.parse, Result.of_const),
        bind(KEnumerated.parse, Result.of_enum),
        bind(KFunction.parse, Result.of_func),
        bind(KView.parse, Result.of_view),
      ])
      >|= Node.map(((name, declaration)) =>
            (export, name, declaration) |> Result.of_export
          );
    }
  );
