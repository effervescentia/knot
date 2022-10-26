open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let closure =
    (parse_stmt: Grammar.Kore.statement_parser_t)
    : Grammar.Kore.expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between(Symbol.open_closure, Symbol.close_closure)
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(AST.Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
