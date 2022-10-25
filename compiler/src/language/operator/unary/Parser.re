open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;

let unary_operation =
    (ctx: ParseContext.t, parse_expr: Grammar.Kore.expression_parser_t)
    : Grammar.Kore.expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse(ctx),
      KAbsolute.Plugin.parse(ctx),
      KNegative.Plugin.parse(ctx),
    ]),
  );
