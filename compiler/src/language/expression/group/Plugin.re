open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.Expression({
  type parse_arg_t = Framework.expression_parser_t;

  type value_t('expr, 'typ) = Node.t('expr, 'typ);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = ((expr_to_xml, _), expression) =>
    Fmt.Node("Group", [], [expr_to_xml(expression)]);
});
