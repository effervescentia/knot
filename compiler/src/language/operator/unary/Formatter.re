open Knot.Kore;
open AST;

let format_operator: Fmt.t(Operator.Unary.t) =
  ppf =>
    Operator.Unary.(
      fun
      | Not => KLogicalNot.Plugin.format
      | Positive => KAbsolute.Plugin.format
      | Negative => KNegative.Plugin.format
    )
    % (pp => pp(ppf, ()));

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Operator.Unary.t, Result.expression_t)) =
  (pp_expression, ppf, (op, (expr, _))) =>
    Fmt.pf(ppf, "%a%a", format_operator, op, pp_expression, expr);
