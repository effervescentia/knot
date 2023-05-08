open Kore;
open AST;

let format: Interface.Plugin.format_t('typ) =
  (ppf, (name, (parameters, (body, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        Lambda.format_parameter_list(Expression.format),
        parameters,
        Lambda.format_body(
          Expression.(
            fun
            | Closure(_) => false
            | _ => true
          ),
          Expression.format,
        ),
        body,
      )
    );
