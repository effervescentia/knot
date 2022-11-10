open Knot.Kore;
open AST;

let pp_function:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((string, list(Result.argument_t), Result.expression_t)) =
  (pp_expression, ppf, (name, args, (expr, _))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s%a@] %a@]",
        name,
        KLambda.Plugin.pp_argument_list(pp_expression),
        args,
        KLambda.Plugin.pp_body(pp_expression),
        expr,
      )
    );
