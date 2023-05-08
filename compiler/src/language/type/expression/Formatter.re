open Knot.Kore;
open AST;

module Keyword = Constants.Keyword;

let rec format: Fmt.t(Interface.t) =
  ppf =>
    Interface.fold(
      ~nil=() => Fmt.string(ppf, Keyword.nil),
      ~boolean=() => Fmt.string(ppf, Keyword.boolean),
      ~integer=() => Fmt.string(ppf, Keyword.integer),
      ~float=() => Fmt.string(ppf, Keyword.float),
      ~string=() => Fmt.string(ppf, Keyword.string),
      ~element=() => Fmt.string(ppf, Keyword.element),
      ~style=() => Fmt.string(ppf, Keyword.style),
      ~identifier=((name, _)) => Fmt.string(ppf, name),
      ~group=((expr, _)) => Fmt.pf(ppf, "(%a)", format, expr),
      ~list=((expr, _)) => Fmt.pf(ppf, "[%a]", format, expr),
      ~object_=
        props =>
          Fmt.(
            closure(
              format_object_property(string),
              ppf,
              props |> List.map(fst),
            )
          ),
      ~function_=
        ((args, (res, _))) =>
          Fmt.(
            pf(
              ppf,
              "(%a) -> %a",
              list(~sep=Sep.comma, format),
              args |> List.map(fst),
              format,
              res,
            )
          ),
      ~dot_access=
        (((root, _), (prop, _))) =>
          Fmt.pf(ppf, "%a.%s", format, root, prop),
      ~view=
        (((prop, _), (res, _))) =>
          Fmt.(
            pf(ppf, "%s(%a, %a)", Keyword.view, format, prop, format, res)
          ),
    )

and format_object_property =
    (pp_key: Fmt.t(string))
    : Fmt.t(Interface.ObjectEntry.t(Interface.node_t)) =>
  ppf =>
    Interface.ObjectEntry.fold(
      ~required=
        (((key, _), (value, _))) =>
          Fmt.(pf(ppf, "%s: %a", key, format, value)),
      ~optional=
        (((key, _), (value, _))) =>
          Fmt.(pf(ppf, "%s?: %a", key, format, value)),
      ~spread=((value, _)) => Fmt.(pf(ppf, "...%a", format, value)),
    );
