open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;
module Symbol = Parse.Symbol;

let closure = (parse_stmt: statement_parser_t): expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between(Symbol.open_closure, Symbol.close_closure)
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(AST.Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
