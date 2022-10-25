open Knot.Kore;

let pp_group: Fmt.t(AST.raw_expression_t) => Fmt.t(AST.expression_t) =
  (pp_expression, ppf, (expr, _)) =>
    switch (expr) {
    | BinaryOp(_) => Fmt.pf(ppf, "(%a)", pp_expression, expr)

    /* collapse parentheses around all other values */
    | _ => pp_expression(ppf, expr)
    };
