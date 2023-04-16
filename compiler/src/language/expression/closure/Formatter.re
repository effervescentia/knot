open Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, pp_expression, ppf) =>
    fun
    | [] => Fmt.string(ppf, "{}")
    | statements =>
      statements
      |> List.map(fst)
      |> Fmt.(closure(Statement.format((), pp_expression), ppf));
