open Knot.Kore;

let pp_operator: Fmt.t(AST.Operator.Unary.t) =
  ppf =>
    AST.Operator.Unary.(
      fun
      | Not => KLogicalNot.Plugin.pp
      | Positive => KAbsolute.Plugin.pp
      | Negative => KNegative.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_unary_operation:
  Fmt.t(AST.Result.raw_expression_t) =>
  Fmt.t((AST.Operator.Unary.t, AST.Result.expression_t)) =
  (pp_expression, ppf, (op, (expr, _))) =>
    Fmt.pf(ppf, "%a%a", pp_operator, op, pp_expression, expr);
