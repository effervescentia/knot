open Knot.Kore;
open AST;

let pp_body: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_expression_t) =
  (pp_expression, ppf) =>
    fun
    | Closure(_) as expr => Fmt.pf(ppf, "-> %a", pp_expression, expr)
    | expr => Fmt.pf(ppf, "-> %a;", pp_expression, expr);

let pp_argument:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_argument_t) =
  (pp_expression, ppf, {name: (name, _), default, _}) =>
    Fmt.pf(
      ppf,
      "%s%a",
      name,
      ppf =>
        fun
        | Some((expr, _)) => Fmt.pf(ppf, " = %a", pp_expression, expr)
        | None => Fmt.nop(ppf, ()),
      default,
    );

let pp_argument_list:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.argument_t)) =
  (pp_expression, ppf) =>
    fun
    | [] => Fmt.nop(ppf, ())
    | args =>
      Fmt.(
        pf(
          ppf,
          "(%a)",
          list(~sep=Sep.trailing_comma, pp_argument(pp_expression)),
          args |> List.map(fst),
        )
      );
