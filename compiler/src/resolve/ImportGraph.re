/**
 Graph that manages the relationships between modules.
 */
open Kore;

type t = {
  imports: Graph.t(m_id),
  get_imports: m_id => list(m_id),
};

let rec add_module = (~added=ref([]), id: m_id, graph: t) => {
  Graph.add_node(id, graph.imports);

  added := added^ |> List.incl(id);

  graph.get_imports(id)
  |> List.iter(child_id => {
       if (!(graph.imports |> Graph.has_node(child_id))) {
         add_module(~added, child_id, graph) |> ignore;
       };

       graph.imports |> Graph.add_edge(id, child_id);
     });

  added^;
};

let create = (entry: m_id, get_imports: m_id => list(m_id)): t => {
  let imports = Graph.empty();
  let graph = {imports, get_imports};

  add_module(entry, graph) |> ignore;

  graph;
};

/* methods */

let rec prune_subtree = (~removed=ref([]), node: 'a, graph: t) => {
  let children = graph.imports |> Graph.get_children(node);

  graph.imports |> Graph.remove_node(node);
  graph.imports |> Graph.remove_edges_from(node);

  removed := removed^ |> List.incl(node);

  children
  |> List.iter(child =>
       if (graph.imports
           |> Graph.get_parents(child)
           |> List.length
           |> (==)(0)) {
         graph |> prune_subtree(~removed, child) |> ignore;
       }
     );

  removed^;
};

let get_modules = graph => graph.imports |> Graph.nodes;

let find_cycles = graph => graph.imports |> Graph.find_all_unique_cycles;

let find_missing = graph =>
  graph.imports
  |> Graph.edges
  |> List.filter_map(
       snd
       % (child => Graph.has_node(child, graph.imports) ? None : Some(child)),
     )
  |> List.uniq_by((==));

let refresh_subtree = (id: m_id, graph: t) => {
  let removed = graph |> prune_subtree(id);
  let added = graph |> add_module(id);

  (removed |> List.filter(id => !List.mem(id, added)), added);
};

let to_string = (graph: t): string =>
  graph.imports |> Graph.to_string(print_m_id);
