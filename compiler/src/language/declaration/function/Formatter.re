open Knot.Kore;
open AST;

let format: Fmt.t((string, (list(Result.argument_t), Result.expression_t))) =
  (ppf, (name, (args, (expr, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        KLambda.Formatter.format_argument_list(KExpression.Plugin.format),
        args,
        KLambda.Formatter.format_body(KExpression.Plugin.format),
        expr,
      )
    );
