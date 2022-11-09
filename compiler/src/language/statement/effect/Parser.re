open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

let effect =
    (ctx: AST.ParseContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  parse_expr(ctx) >|= Node.wrap(AST.Raw.of_expr);
