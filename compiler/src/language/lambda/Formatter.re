open Knot.Kore;
open AST;

let format_body: ('expr => bool, Fmt.t('expr)) => Fmt.t('expr) =
  (add_terminator, pp_expression, ppf, expr) =>
    if (add_terminator(expr)) {
      Fmt.pf(ppf, "-> %a;", pp_expression, expr);
    } else {
      Fmt.pf(ppf, "-> %a", pp_expression, expr);
    };

let format_parameter:
  Fmt.t('expr) => Fmt.t(Interface.Parameter.t('expr, 'typ)) =
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
  Fmt.t('expr) => Fmt.t(list(Interface.Parameter.node_t('expr, 'typ))) =
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
