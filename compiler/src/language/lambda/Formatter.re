open Knot.Kore;
open AST;

let format_body:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_expression_t) =
  (pp_expression, ppf) =>
    fun
    | Closure(_) as expr => Fmt.pf(ppf, "-> %a", pp_expression, expr)
    | expr => Fmt.pf(ppf, "-> %a;", pp_expression, expr);

let format_parameter:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_parameter_t) =
  (pp_expression, ppf, ((name, _), _, default)) =>
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

let format_parameter_list:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.parameter_t)) =
  (pp_expression, ppf) =>
    fun
    | [] => Fmt.nop(ppf, ())
    | args =>
      Fmt.(
        pf(
          ppf,
          "(%a)",
          list(~sep=Sep.trailing_comma, format_parameter(pp_expression)),
          args |> List.map(fst),
        )
      );
