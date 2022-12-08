open Parse.Kore;
open AST;

let parse =
    (parse_expr: Framework.expression_parser_t): Framework.expression_parser_t =>
  Matchers.unary_op(
    parse_expr,
    choice([
      KLogicalNot.Plugin.parse,
      KAbsolute.Plugin.parse,
      KNegative.Plugin.parse,
    ]),
  );
