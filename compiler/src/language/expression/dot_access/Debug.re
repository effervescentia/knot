open Knot.Kore;
open AST;

let to_xml:
  (
    Expression.expression_t('a) => Fmt.xml_t(string),
    (Expression.expression_t('a), Common.untyped_t(string))
  ) =>
  Fmt.xml_t(string) =
  (expr_to_xml, (root, property)) =>
    Node(
      "DotAccess",
      [],
      [
        Node("Object", [], [expr_to_xml(root)]),
        Dump.node_to_xml(~dump_value=Fun.id, "Property", property),
      ],
    );
