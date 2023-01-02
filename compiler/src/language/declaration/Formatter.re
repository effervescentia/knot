open Kore;

let format: Fmt.t((string, AST.Module.raw_declaration_t)) =
  (ppf, (name, declaration)) => {
    let bind = format => Tuple.with_fst2(name) % format(ppf);

    declaration
    |> Util.fold(
         ~constant=bind(KConstant.format),
         ~enumerated=bind(KEnumerated.format),
         ~function_=bind(KFunction.format),
         ~view=bind(KView.format),
       );
  };
