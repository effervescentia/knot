open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;

let unary_operation = (parse_expr: expression_parser_t): expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
