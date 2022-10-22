open Knot.Kore;

let parse = Parser.function_;

let pp:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((string, list(AST.argument_t), AST.expression_t)) =
  (pp_expression, ppf, (name, args, (expr, _))) =>
    Fmt.(
      pf(
        ppf,
        "@[<v>func @[<h>%s(%a)@] -> %a@]",
        name,
        KLambda.Plugin.pp_argument_list(pp_expression),
        args,
        KLambda.Plugin.pp_body(pp_expression),
        expr,
      )
    );
