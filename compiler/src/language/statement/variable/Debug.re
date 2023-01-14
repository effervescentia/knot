open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, (name, expression)) =>
    Node(
      "Variable",
      [],
      [
        Dump.identifier_to_xml("Name", name),
        Node("Value", [], [expr_to_xml(expression)]),
      ],
    );
