open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, pp_expression, ppf, ((function_, _), arguments)) =>
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
