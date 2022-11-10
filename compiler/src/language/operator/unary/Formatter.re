open Knot.Kore;
open AST;

let pp_operator: Fmt.t(Operator.Unary.t) =
  ppf =>
    Operator.Unary.(
      fun
      | Not => KLogicalNot.Plugin.pp
      | Positive => KAbsolute.Plugin.pp
      | Negative => KNegative.Plugin.pp
    )
    % (pp => pp(ppf, ()));

let pp_unary_operation:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Operator.Unary.t, Result.expression_t)) =
  (pp_expression, ppf, (op, (expr, _))) =>
    Fmt.pf(ppf, "%a%a", pp_operator, op, pp_expression, expr);
