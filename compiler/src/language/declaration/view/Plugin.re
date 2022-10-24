open Knot.Kore;

let parse = Parser.view;

let pp:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t(
    (
      string,
      list(AST.argument_t),
      list(AST.node_t(string)),
      AST.expression_t,
    ),
  ) =
  (pp_expression, ppf, (name, args, mixins, (expr, _))) =>
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
