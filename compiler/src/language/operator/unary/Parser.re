open Parse.Kore;
open AST;

let unary_operation =
    (parse_expr: ParserTypes.expression_parser_t)
    : ParserTypes.expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
