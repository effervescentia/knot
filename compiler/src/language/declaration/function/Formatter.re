open Knot.Kore;
open AST;

let format: Interface.Plugin.format_t('typ) =
  (ppf, (name, (parameters, (body, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        KLambda.Formatter.format_parameter_list(KExpression.Plugin.format),
        parameters,
        KLambda.Formatter.format_body(
          KExpression.Interface.(
            fun
            | Closure(_) => false
            | _ => true
          ),
          KExpression.Plugin.format,
        ),
        body,
      )
    );
