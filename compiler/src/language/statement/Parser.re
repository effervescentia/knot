open Knot.Kore;
open Parse.Kore;
open AST;

let statement =
    (
      ctx: ParseContext.t,
      parse_expr: ParserTypes.contextual_expression_parser_t,
    )
    : ParserTypes.statement_parser_t =>
  choice([
    KVariable.Plugin.parse(ctx, parse_expr),
    KEffect.Plugin.parse(ctx, parse_expr),
  ])
  |> Matchers.terminated;
