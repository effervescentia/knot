open Knot.Kore;
open Parse.Kore;

let parse = ((parse_term, parse_expression)) => {
  let rec loop = expression =>
    parse_expression
    |> Matchers.comma_sep
    |> Matchers.between_parentheses
    >>= (
      arguments =>
        loop(
          Node.raw(
            (expression, fst(arguments)) |> AST.Raw.of_func_call,
            Node.get_range(arguments),
          ),
        )
    )
    |> option(expression);

  parse_term >>= loop;
};
