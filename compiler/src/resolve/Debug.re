open Kore;

let print_import_graph = (graph: ImportGraph.t): string =>
  graph.imports |> Graph.to_string(Reference.Namespace.to_string);
