open Knot.Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  (expr_to_xml, (kind, view, style)) =>
    Fmt.Node(
      "BindStyle",
      [("kind", KSX.Interface.ViewKind.to_string(kind))],
      [
        Node("View", [], [expr_to_xml(view)]),
        Node("Style", [], [expr_to_xml(style)]),
      ],
    );
