open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = Raw.expression_t;

  type value_t('expr, 'typ) = (Node.t('expr, 'typ), Common.identifier_t);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
