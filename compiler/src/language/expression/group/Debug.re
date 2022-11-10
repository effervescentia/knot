open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    Expression.expression_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, expr) => Node("Group", [], [expr_to_xml(expr)]);
