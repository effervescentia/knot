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
      let (&>) = (parse, to_declaration) =>
        parse(arg)
        >|= Node.map(((name, declaration)) =>
              (export, name, declaration |> Node.map(to_declaration))
              |> Result.of_export
            );

      choice([
        KConstant.parse &> Result.of_const,
        KEnumerated.parse &> Result.of_enum,
        KFunction.parse &> Result.of_func,
        KView.parse &> Result.of_view,
      ]);
    }
  );
