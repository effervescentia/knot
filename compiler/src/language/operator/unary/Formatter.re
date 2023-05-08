open Kore;

let format_operator: Fmt.t(AST.Operator.Unary.t) =
  ppf =>
    AST.Operator.Unary.(
      fun
      | Not => KLogicalNot.format
      | Positive => KAbsolute.format
      | Negative => KNegative.format
    )
    % (pp => pp(ppf, ()));

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, pp_expression, ppf, (operator, (expression, _))) =>
    Fmt.pf(ppf, "%a%a", format_operator, operator, pp_expression, expression);
