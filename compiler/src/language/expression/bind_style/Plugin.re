open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_bind_style;

let pp = Formatter.pp_bind_style;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    Framework.contextual_expression_parser_t,
  );

  type value_t('a) = (
    Expression.expression_t('a),
    Expression.expression_t('a),
  );

  let parse = Parser.bind_style;

  let format = pp;

  let to_xml = Debug.to_xml;
});
