open Knot.Kore;

let format = (pp_expression, ppf, ((function_, _), arguments)) =>
  Fmt.(
    pf(
      ppf,
      "%a@[<hv>(%a)@]",
      pp_expression,
      function_,
      list(~sep=Sep.trailing_comma, pp_expression),
      arguments |> List.map(fst),
    )
  );
