open Knot.Kore;

let to_xml = ((expr_to_xml, _), (name, args)) =>
  Fmt.Node(
    "FunctionCall",
    [],
    [
      Node("Name", [], [expr_to_xml(name)]),
      Node("Arguments", [], args |> List.map(expr_to_xml)),
    ],
  );
