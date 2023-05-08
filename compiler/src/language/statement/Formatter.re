open Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  ((), pp_expression, ppf) => {
    let bind = format => format(pp_expression, ppf);

    Interface.fold(
      ~variable=bind(Variable.format),
      ~effect=bind(Effect.format),
    );
  };
