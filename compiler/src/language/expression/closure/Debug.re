open Knot.Kore;
open AST;

let to_xml = ((expr_to_xml, dump_type), stmts) =>
  Fmt.Node(
    "Closure",
    [],
    stmts |> List.map(KStatement.Plugin.to_xml(expr_to_xml, dump_type)),
  );
