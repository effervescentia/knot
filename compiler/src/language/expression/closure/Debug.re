open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, dump_type), statements) =>
  Fmt.Node(
    "Closure",
    [],
    statements |> List.map(KStatement.Plugin.to_xml(expr_to_xml, dump_type)),
  );
