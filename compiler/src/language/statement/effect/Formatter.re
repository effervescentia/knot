open Knot.Kore;
open AST;

let format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.expression_t) =
  (pp_expression, ppf, (expression, _)) =>
    Fmt.pf(ppf, "%a;", pp_expression, expression);
