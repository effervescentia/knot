open Knot.Kore;
open AST;

let analyze = Analyzer.analyze_group;

include Framework.Expression({
  type parse_arg_t = ParserTypes.expression_parser_t;
  type pp_arg_t = Fmt.t(Result.raw_expression_t);

  type value_t('a) = Expression.expression_t('a);

  let parse = Parser.group;

  let pp = Formatter.pp_group;

  let to_xml = ((expr_to_xml, _), expr) =>
    Fmt.Node("Group", [], [expr_to_xml(expr)]);
});
