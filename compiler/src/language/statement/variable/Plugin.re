open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Statement({
  type value_t('a) = (Common.identifier_t, Expression.expression_t('a));

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
