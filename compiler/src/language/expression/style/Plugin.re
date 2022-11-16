open Knot.Kore;
open AST;

let parse_style_literal = Parser.style_literal;

let analyze = Analyzer.analyze_style;

let pp = Formatter.pp_style_expression;
let pp_style_rules = Formatter.pp_style_rules;

include Framework.Expression({
  type parse_arg_t = (
    ParseContext.t,
    Framework.contextual_expression_parser_t,
  );

  type value_t('a) = list(Expression.style_rule_t('a));

  let parse = Parser.style_expression;

  let format = pp;

  let to_xml = Debug.to_xml;
});
