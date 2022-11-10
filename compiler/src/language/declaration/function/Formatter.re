open Knot.Kore;
open AST;

let pp_function:
  Fmt.t((string, (list(Result.argument_t), Result.expression_t))) =
  (ppf, (name, (args, (expr, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        KLambda.Plugin.pp_argument_list(KExpression.Plugin.pp),
        args,
        KLambda.Plugin.pp_body(KExpression.Plugin.pp),
        expr,
      )
    );
