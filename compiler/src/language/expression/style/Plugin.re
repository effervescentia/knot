open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_style;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    ParserTypes.contextual_expression_parser_t,
  );

  type value_t('a) = list(Expression.style_rule_t('a));

  let parse = Parser.style_expression;

  let format = Formatter.pp_style_expression;

  let to_xml = Debug.to_xml;
});
