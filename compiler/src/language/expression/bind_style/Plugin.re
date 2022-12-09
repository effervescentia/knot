open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    Framework.contextual_expression_parser_t,
  );

  type value_t('a) = (
    Expression.bind_style_target_t('a),
    Expression.expression_t('a),
  );

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = Debug.to_xml;
});
