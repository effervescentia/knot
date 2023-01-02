open Knot.Kore;
open AST;

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.expression_t, Result.untyped_t(string))) =
  (pp_expression, ppf, ((object_, _), (property, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, object_, property);
