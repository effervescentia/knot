open Knot.Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, pp_expression, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | statements =>
      statements
      |> List.map(fst)
      |> Fmt.(closure(KStatement.Plugin.format((), pp_expression), ppf));
