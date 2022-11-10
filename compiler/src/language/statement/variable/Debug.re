open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (Common.untyped_t(string), Expression.expression_t('a))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (name, expr)) =>
    Node(
      "Variable",
      [],
      [
        Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
        Node("Value", [], [expr_to_xml(expr)]),
      ],
    );
