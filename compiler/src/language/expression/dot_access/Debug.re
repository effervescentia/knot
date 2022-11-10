open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (root, property)) =>
  Fmt.Node(
    "DotAccess",
    [],
    [
      Node("Object", [], [expr_to_xml(root)]),
      Dump.node_to_xml(~dump_value=Fun.id, "Property", property),
    ],
  );
