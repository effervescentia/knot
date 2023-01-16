open Knot.Kore;
open AST;

let format:
  Fmt.t(
    (
      string,
      (list(Result.parameter_t), KExpression.Interface.node_t('typ)),
    ),
  ) =
  (ppf, (name, (parameters, (body, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        KLambda.Formatter.format_parameter_list(KExpression.Plugin.format),
        parameters,
        KLambda.Formatter.format_body(KExpression.Plugin.format),
        body,
      )
    );
