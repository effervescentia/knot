open Knot.Kore;
open AST;

let pp_view:
  Fmt.t(
    (
      string,
      (
        list(Result.argument_t),
        list(Result.node_t(string)),
        Result.expression_t,
      ),
    ),
  ) =
  (ppf, (name, (args, _, (expr, _)))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>view @[<h>%s%a@] %a@]",
        name,
        KLambda.Plugin.pp_argument_list(KExpression.Plugin.pp),
        args,
        KLambda.Plugin.pp_body(KExpression.Plugin.pp),
        expr,
      )
    );
