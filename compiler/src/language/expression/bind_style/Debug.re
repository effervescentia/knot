open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (kind, view, style)) =>
  Fmt.Node(
    "BindStyle",
    [("kind", KSX.ViewKind.to_string(kind))],
    [
      Node("View", [], [expr_to_xml(view)]),
      Node("Style", [], [expr_to_xml(style)]),
    ],
  );
