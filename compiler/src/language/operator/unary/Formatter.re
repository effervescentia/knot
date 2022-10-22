open Knot.Kore;

let pp_operator: Fmt.t(AST.unary_t) =
  ppf =>
    (
      fun
      | AST.Not => KLogicalNot.Plugin.pp
      | AST.Positive => KAbsolute.Plugin.pp
      | AST.Negative => KNegative.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_unary_operation:
  Fmt.t(AST.raw_expression_t) => Fmt.t((AST.unary_t, AST.expression_t)) =
  (pp_expression, ppf, (op, (expr, _))) =>
    Fmt.pf(ppf, "%a%a", pp_operator, op, pp_expression, expr);
