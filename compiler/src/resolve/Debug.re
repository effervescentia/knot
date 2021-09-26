open Kore;

let print_import_graph = (graph: ImportGraph.t): string =>
  graph.imports |> ~@Graph.pp(Reference.Namespace.pp);
