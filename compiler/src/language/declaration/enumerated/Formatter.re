open Knot.Kore;
open AST;

let format_argument_list =
  Fmt.(
    (ppf, args) =>
      List.is_empty(args)
        ? () : pf(ppf, "(%a)", list(KTypeExpression.Plugin.format), args)
  );

let format_variant_list =
  Fmt.(
    block(~layout=Vertical, ~sep=Sep.space, (ppf, ((arg_name, _), args)) =>
      pf(
        ppf,
        "@[<h>| %s%a@]",
        arg_name,
        format_argument_list,
        args |> List.map(fst),
      )
    )
  );

let format:
  Fmt.t(
    (
      string,
      list(
        (Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))),
      ),
    ),
  ) =
  ppf =>
    fun
    | (name, []) => Fmt.(pf(ppf, "enum %s = | ;", name))
    | (name, variants) =>
      Fmt.(
        pf(ppf, "@[<v>enum %s =%a;@]", name, format_variant_list, variants)
      );
