open Knot.Kore;
open AST;

let to_xml:
  ('expr => Fmt.xml_t(string), (Common.identifier_t, 'expr)) =>
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
