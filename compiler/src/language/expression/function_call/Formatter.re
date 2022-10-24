open Knot.Kore;

let pp_function_call:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.expression_t, list(AST.expression_t))) =
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