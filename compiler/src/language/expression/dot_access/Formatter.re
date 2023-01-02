open Knot.Kore;
open AST;

let format:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.expression_t, Common.identifier_t)) =
  (pp_expression, ppf, ((object_, _), (property, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, object_, property);
