open Parse.Onyx;

module Matchers = Grammar.Matchers;

let unary_operation =
    (parse_expr: Grammar.Kore.expression_parser_t)
    : Grammar.Kore.expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
