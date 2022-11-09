open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module ParseContext = AST.ParseContext;

let statement =
    (ctx: ParseContext.t, parse_expr: contextual_expression_parser_t)
    : statement_parser_t =>
  choice([
    KVariable.Plugin.parse(ctx, parse_expr),
    KEffect.Plugin.parse(ctx, parse_expr),
  ])
  |> Matchers.terminated;
