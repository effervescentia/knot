open Knot.Kore;
open AST;

let pp_enumerated:
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
                  ? ()
                  : pf(ppf, "(%a)", list(KTypeExpression.Plugin.pp), args),
              args |> List.map(fst),
            )
          ),
          variants,
        )
      );
