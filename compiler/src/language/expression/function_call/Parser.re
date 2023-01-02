open Knot.Kore;
open Parse.Kore;

let parse = ((parse_term, parse_expression)) => {
  let rec loop = expr =>
    parse_expression
    |> Matchers.comma_sep
    |> Matchers.between_parentheses
    >>= (
      args =>
        loop(
          Node.raw(
            (expr, fst(args)) |> AST.Raw.of_func_call,
            Node.get_range(args),
          ),
        )
    )
    |> option(expr);

  parse_term >>= loop;
};
