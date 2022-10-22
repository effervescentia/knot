open Knot.Kore;
open Parse.Onyx;

let effect =
    (
      ctx: ParseContext.t,
      parse_expr: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.statement_parser_t =>
  parse_expr(ctx) >|= Node.wrap(AST.Raw.of_expr);
