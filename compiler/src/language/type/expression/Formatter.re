open Knot.Kore;
open AST;

let rec format: Fmt.t(TypeExpression.raw_t) =
  ppf =>
    fun
    | Nil => Fmt.string(ppf, Constants.Keyword.nil)
    | Boolean => Fmt.string(ppf, Constants.Keyword.boolean)
    | Integer => Fmt.string(ppf, Constants.Keyword.integer)
    | Float => Fmt.string(ppf, Constants.Keyword.float)
    | String => Fmt.string(ppf, Constants.Keyword.string)
    | Element => Fmt.string(ppf, Constants.Keyword.element)
    | Style => Fmt.string(ppf, Constants.Keyword.style)
    | Identifier((name, _)) => Fmt.string(ppf, name)
    | Group((expr, _)) => Fmt.pf(ppf, "(%a)", format, expr)
    | List((expr, _)) => Fmt.pf(ppf, "[%a]", format, expr)

    | Object(props) =>
      Fmt.(
        closure(format_struct_property(string), ppf, props |> List.map(fst))
      )

    | Function(args, (res, _)) =>
      Fmt.(
        pf(
          ppf,
          "(%a) -> %a",
          list(~sep=Sep.comma, format),
          args |> List.map(fst),
          format,
          res,
        )
      )

    | DotAccess((root, _), (prop, _)) =>
      Fmt.pf(ppf, "%a.%s", format, root, prop)

    | View((prop, _), (res, _)) =>
      Fmt.(
        pf(
          ppf,
          "%s(%a, %a)",
          Constants.Keyword.view,
          format,
          prop,
          format,
          res,
        )
      )

and format_struct_property =
    (pp_key: Fmt.t(string)): Fmt.t(TypeExpression.raw_object_entry_t) =>
  ppf =>
    fun
    | Required((key, _), (value, _)) =>
      Fmt.(pf(ppf, "%s: %a", key, format, value))
    | Optional((key, _), (value, _)) =>
      Fmt.(pf(ppf, "%s?: %a", key, format, value))
    | Spread((value, _)) => Fmt.(pf(ppf, "...%a", format, value));
