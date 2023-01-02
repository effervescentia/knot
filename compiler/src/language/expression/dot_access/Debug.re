open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (object_, property)) =>
  Fmt.Node(
    "DotAccess",
    [],
    [
      Node("Object", [], [expr_to_xml(object_)]),
      Dump.identifier_to_xml("Property", property),
    ],
  );
