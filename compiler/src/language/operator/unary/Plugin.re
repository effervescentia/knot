open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_unary_operation;

let pp = Formatter.pp_unary_operation;

include Framework.Expression({
  type parse_arg_t = Framework.expression_parser_t;

  type value_t('a) = (Operator.Unary.t, Expression.expression_t('a));

  let parse = Parser.unary_operation;

  let format = pp;

  let to_xml = Debug.to_xml;
});
