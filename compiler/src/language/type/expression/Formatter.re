open Knot.Kore;

let rec pp_type_expr: Fmt.t(AST.TypeExpression.raw_t) =
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
    | Group((expr, _)) => Fmt.pf(ppf, "(%a)", pp_type_expr, expr)
    | List((expr, _)) => Fmt.pf(ppf, "[%a]", pp_type_expr, expr)
    | Struct(props) =>
      Fmt.(
        record(
          string,
          pp_type_expr,
          ppf,
          props |> List.map(Tuple.map_each2(fst, fst)),
        )
      )
    | Function(args, (res, _)) =>
      Fmt.(
        pf(
          ppf,
          "(%a) -> %a",
          list(~sep=Sep.comma, pp_type_expr),
          args |> List.map(fst),
          pp_type_expr,
          res,
        )
      )
    | DotAccess((root, _), (prop, _)) =>
      Fmt.pf(ppf, "%a.%s", pp_type_expr, root, prop);