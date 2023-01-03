open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Statement({
  type value_t('expr, 'typ) = (Common.identifier_t, 'expr);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
