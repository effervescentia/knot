open Knot.Kore;
open AST;

let analyze = Analyzer.analyze;

include Framework.StatementV2({
  type value_t('a) = Expression.expression_t('a);

  let parse = Parser.parse;

  let format = Formatter.format;

  let to_xml = (expr_to_xml, expr) =>
    Fmt.Node("Effect", [], [expr_to_xml(expr)]);
});
