open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.StatementV2({
  type value_t('a) = (
    Common.untyped_t(string),
    Expression.expression_t('a),
  );

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
