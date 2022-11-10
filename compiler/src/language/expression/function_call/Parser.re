open Knot.Kore;
open Parse.Kore;
open AST;

let function_call =
    (
      parse_term: ParserTypes.expression_parser_t,
      parse_expr: ParserTypes.expression_parser_t,
    ) => {
  let rec loop = expr =>
    parse_expr
    |> Matchers.comma_sep
    |> Matchers.between_parentheses
    >>= (
      args =>
        loop(
          Node.typed(
            (expr, fst(args)) |> Raw.of_func_call,
            (),
            Node.get_range(args),
          ),
        )
    )
    |> option(expr);

  parse_term >>= loop;
};
