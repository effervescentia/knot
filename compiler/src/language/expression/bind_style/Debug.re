open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (tag_type, view, style)) =>
  Fmt.Node(
    "BindStyle",
    [],
    [
      Node("View", [], [expr_to_xml(view)]),
      Node("Style", [], [expr_to_xml(style)]),
    ],
  );
