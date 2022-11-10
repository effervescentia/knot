open Knot.Kore;
open Parse.Kore;
open AST;

let effect =
    (
      (
        ctx: ParseContext.t,
        parse_expr: ParserTypes.contextual_expression_parser_t,
      ),
    )
    : ParserTypes.statement_parser_t =>
  parse_expr(ctx) >|= Node.wrap(Raw.of_expr);
