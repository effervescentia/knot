open Knot.Kore;
open Parse.Kore;

let parse = ((parse_term, parse_expr)) => {
  let rec loop = expr =>
    parse_expr
    |> Matchers.comma_sep
    |> Matchers.between_parentheses
    >>= (
      args =>
        loop(
          Node.typed(
            (expr, fst(args)) |> AST.Raw.of_func_call,
            (),
            Node.get_range(args),
          ),
        )
    )
    |> option(expr);

  parse_term >>= loop;
};
