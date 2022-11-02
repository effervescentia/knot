open Knot.Kore;

let pp_operator: Fmt.t(AST.Result.unary_t) =
  ppf =>
    (
      fun
      | AST.Result.Not => KLogicalNot.Plugin.pp
      | AST.Result.Positive => KAbsolute.Plugin.pp
      | AST.Result.Negative => KNegative.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_unary_operation:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t((AST.Result.unary_t, AST.Result.expression_t)) =
  (pp_expression, ppf, (op, (expr, _))) =>
    Fmt.pf(ppf, "%a%a", pp_operator, op, pp_expression, expr);
