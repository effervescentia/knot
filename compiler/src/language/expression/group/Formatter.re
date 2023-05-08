open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (is_binary_op, pp_expression, ppf, (expression, _)) =>
    if (is_binary_op(expression)) {
      Fmt.pf(ppf, "(%a)", pp_expression, expression);
    } else {
      /* collapse parentheses around all other values */
      pp_expression(
        ppf,
        expression,
      );
    };
