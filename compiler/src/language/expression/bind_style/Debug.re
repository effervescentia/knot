open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, _), (view, style)) =>
  Fmt.Node(
    "BindStyle",
    [],
    [
      Node(
        "View",
        [],
        [
          switch (view) {
          | Expression.BuiltIn(view')
          | Expression.Local(view') => expr_to_xml(view')
          },
        ],
      ),
      Node("Style", [], [expr_to_xml(style)]),
    ],
  );
