open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;

let statement =
    (
      ctx: ParseContext.t,
      parse_expr: Parse.Kore.contextual_expression_parser_t,
    )
    : Parse.Kore.statement_parser_t =>
  choice([
    KVariable.Plugin.parse(ctx, parse_expr),
    KEffect.Plugin.parse(ctx, parse_expr),
  ])
  |> Matchers.terminated;
