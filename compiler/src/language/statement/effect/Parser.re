open Knot.Kore;
open Parse.Kore;
open AST;

let parse =
    (
      (
        ctx: ParseContext.t,
        parse_expr: Framework.contextual_expression_parser_t,
      ),
    )
    : Framework.statement_parser_t =>
  parse_expr(ctx) >|= Node.wrap(Raw.of_effect);
