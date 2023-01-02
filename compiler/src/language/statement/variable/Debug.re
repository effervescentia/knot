open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (Common.identifier_t, Expression.expression_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (name, expression)) =>
    Node(
      "Variable",
      [],
      [
        Dump.identifier_to_xml("Name", name),
        Node("Value", [], [expr_to_xml(expression)]),
      ],
    );
