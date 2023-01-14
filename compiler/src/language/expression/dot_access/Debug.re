open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, (object_, property)) =>
    Fmt.Node(
      "DotAccess",
      [],
      [
        Node("Object", [], [expr_to_xml(object_)]),
        Dump.identifier_to_xml("Property", property),
      ],
    );
