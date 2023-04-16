open Kore;
open AST;

let format: Fmt.t((string, Interface.t('typ))) =
  (ppf, (name, declaration)) => {
    let bind = format => Tuple.with_fst2(name) % format(ppf);

    declaration
    |> Interface.fold(
         ~constant=bind(Constant.format),
         ~enumerated=bind(Enumerated.format),
         ~function_=bind(Function.format),
         ~view=bind(View.format),
       );
  };
