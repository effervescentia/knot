open Knot.Kore;
open AST;

let format:
  Fmt.t(
    (
      string,
      (
        list(Result.parameter_t),
        list(Result.node_t(string)),
        Result.expression_t,
      ),
    ),
  ) =
  (ppf, (name, (parameters, _, (body, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>view @[<h>%s%a@] %a@]",
        name,
        KLambda.Formatter.format_parameter_list(KExpression.Plugin.format),
        parameters,
        KLambda.Formatter.format_body(KExpression.Plugin.format),
        body,
      )
    );
