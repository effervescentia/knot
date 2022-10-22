open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;

let statement =
    (
      ctx: ParseContext.t,
      parse_expr: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.statement_parser_t =>
  choice([
    KVariable.Plugin.parse(ctx, parse_expr),
    KEffect.Plugin.parse(ctx, parse_expr),
  ])
  |> Matchers.terminated;
