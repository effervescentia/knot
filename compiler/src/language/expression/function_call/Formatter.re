open Knot.Kore;
open AST;

let pp_function_call:
  Fmt.t(Result.raw_expression_t) =>
  Fmt.t((Result.expression_t, list(Result.expression_t))) =
  (pp_expression, ppf, ((expr, _), args)) =>
    Fmt.(
      pf(
        ppf,
        "%a@[<hv>(%a)@]",
        pp_expression,
        expr,
        list(~sep=Sep.trailing_comma, pp_expression),
        args |> List.map(fst),
      )
    );
