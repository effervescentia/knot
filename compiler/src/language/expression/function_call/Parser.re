open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module Symbol = Parse.Symbol;

let function_call =
    (
      parse_term: Parse.Kore.expression_parser_t,
      parse_expr: Parse.Kore.expression_parser_t,
    ) => {
  let rec loop = expr =>
    parse_expr
    |> Matchers.comma_sep
    |> Matchers.between(Symbol.open_group, Symbol.close_group)
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
