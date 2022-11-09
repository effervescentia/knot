open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

let closure = (parse_stmt: statement_parser_t): expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between_braces
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(AST.Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
