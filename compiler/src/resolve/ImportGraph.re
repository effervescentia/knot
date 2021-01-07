open Kore;

type t = {
  imports: Graph.t(m_id),
  get_imports: m_id => list(m_id),
};

let rec add_module = (id: m_id, graph: t) => {
  Graph.add_node(id, graph.imports);

  graph.get_imports(id)
  |> List.iter(child_id => {
       if (!(graph.imports |> Graph.has_node(child_id))) {
         add_module(child_id, graph);
       };

       graph.imports |> Graph.add_edge(id, child_id);
     });
};

let remove_subtree = (x, graph) => graph.imports |> Graph.remove_subtree(x);

let get_modules = graph => graph.imports |> Graph.get_nodes;

let find_cycles = graph => graph.imports |> Graph.find_all_unique_cycles;

let refresh_subtree = (id: m_id, graph: t) => {
  graph.imports |> Graph.remove_subtree(id);
  graph.imports |> Graph.add_node(id);
};

let create = (entry: m_id, get_imports: m_id => list(m_id)): t => {
  let imports = Graph.empty();
  let graph = {imports, get_imports};

  add_module(entry, graph);

  graph;
};

let to_string = (graph: t): string =>
  graph.imports |> Graph.to_string(print_m_id);
