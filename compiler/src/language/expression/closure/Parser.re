open Knot.Kore;
open Parse.Kore;
open AST;

let closure =
    (parse_stmt: ParserTypes.statement_parser_t)
    : ParserTypes.expression_parser_t =>
  parse_stmt
  |> many
  |> Matchers.between_braces
  >|= (
    ((stmts, _) as stmts_node) =>
      Node.typed(Raw.of_closure(stmts), (), Node.get_range(stmts_node))
  );
