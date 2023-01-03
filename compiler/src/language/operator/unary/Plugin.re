open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = Framework.expression_parser_t;

  type value_t('expr, 'typ) = (Operator.Unary.t, Node.t('expr, 'typ));

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
