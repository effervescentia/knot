open Knot.Kore;

let pp_view:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t(
    (
      string,
      list(AST.Result.argument_t),
      list(AST.Result.node_t(string)),
      AST.Result.expression_t,
    ),
  ) =
  (pp_expression, ppf, (name, args, _, (expr, _))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>view @[<h>%s%a@] %a@]",
        name,
        KLambda.Plugin.pp_argument_list(pp_expression),
        args,
        KLambda.Plugin.pp_body(pp_expression),
        expr,
      )
    );
