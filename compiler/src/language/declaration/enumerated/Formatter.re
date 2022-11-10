open Knot.Kore;
open AST;

let pp_enumerated:
  Fmt.t(TypeExpression.raw_t) =>
  Fmt.t(
    (
      string,
      list(
        (Result.identifier_t, list(Result.node_t(TypeExpression.raw_t))),
      ),
    ),
  ) =
  (pp_type_expr, ppf) =>
    fun
    | (name, []) => Fmt.(pf(ppf, "enum %s = | ;", name))
    | (name, variants) =>
      Fmt.(
        pf(
          ppf,
          "@[<v>enum %s =%a;@]",
          name,
          block(
            ~layout=Vertical, ~sep=Sep.space, (ppf, ((arg_name, _), args)) =>
            pf(
              ppf,
              "@[<h>| %s%a@]",
              arg_name,
              (ppf, args) =>
                List.is_empty(args)
                  ? () : pf(ppf, "(%a)", list(pp_type_expr), args),
              args |> List.map(fst),
            )
          ),
          variants,
        )
      );
