open Parse.Kore;
open AST.ParserTypes;

let unary_operation = (parse_expr: expression_parser_t): expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
