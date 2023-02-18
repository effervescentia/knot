open Kore;
open AST;

let format: Fmt.t((string, Interface.t('typ))) =
  (ppf, (name, declaration)) => {
    let bind = format => Tuple.with_fst2(name) % format(ppf);

    declaration
    |> Interface.fold(
         ~constant=bind(KConstant.format),
         ~enumerated=bind(KEnumerated.format),
         ~function_=bind(KFunction.format),
         ~view=bind(KView.format),
       );
  };
