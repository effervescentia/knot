open Knot.Kore;

let to_xml:
  (
    AST.Expression.expression_t('a) => Fmt.xml_t(string),
    AST.Expression.expression_t('a)
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, expression) =>
    Node("Effect", [], [expr_to_xml(expression)]);
