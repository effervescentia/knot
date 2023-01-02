open Knot.Kore;

let to_xml = ((expr_to_xml, _), (function_, arguments)) =>
  Fmt.Node(
    "FunctionCall",
    [],
    [
      Node("Function", [], [expr_to_xml(function_)]),
      Node("Arguments", [], arguments |> List.map(expr_to_xml)),
    ],
  );
