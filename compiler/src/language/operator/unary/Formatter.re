open Kore;
open AST;

let format_operator: Fmt.t(Operator.Unary.t) =
  ppf =>
    Operator.Unary.(
      fun
      | Not => KLogicalNot.format
      | Positive => KAbsolute.format
      | Negative => KNegative.format
    )
    % (pp => pp(ppf, ()));

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Operator.Unary.t, Result.expression_t)) =
  (pp_expression, ppf, (operator, (expression, _))) =>
    Fmt.pf(ppf, "%a%a", format_operator, operator, pp_expression, expression);
