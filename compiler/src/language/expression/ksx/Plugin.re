open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = (ParseContext.t, Parser.expression_parsers_arg_t);

  type value_t('expr, 'typ) = AST.KSX.t('expr, 'typ);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
