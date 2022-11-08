open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let closure = (parse_stmt: statement_parser_t): expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between_braces
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(AST.Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
