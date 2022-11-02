open Parse.Onyx;

module Matchers = Parse.Matchers;

let unary_operation =
    (parse_expr: Parse.Kore.expression_parser_t)
    : Parse.Kore.expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
