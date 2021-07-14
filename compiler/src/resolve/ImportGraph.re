open Kore;
open Reference;

/**
 graph that manages the relationships between modules
 */
type t = {
  imports: Graph.t(Namespace.t),
  get_imports: Namespace.t => list(Namespace.t),
};

/* static */

let create = (get_imports: Namespace.t => list(Namespace.t)): t => {
  imports: Graph.empty(),
  get_imports,
};

/* methods */

let rec add_module = (~added=ref([]), id: Namespace.t, graph: t) => {
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

let init = (entry: Namespace.t) => add_module(entry) % ignore;

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

let get_modules = (graph: t) => graph.imports |> Graph.nodes;

let get_imported_by = (entry: Namespace.t, graph: t) =>
  graph.imports |> Graph.get_children(entry);

let has_module = (entry: Namespace.t, graph: t) =>
  graph.imports |> Graph.has_node(entry);

let find_cycles = (graph: t) => graph.imports |> Graph.find_all_unique_cycles;

let find_missing = (graph: t) =>
  graph.imports
  |> Graph.edges
  |> List.filter_map(
       snd
       % (child => Graph.has_node(child, graph.imports) ? None : Some(child)),
     )
  |> List.uniq_by((==));

let refresh_subtree = (id: Namespace.t, graph: t) => {
  let removed = graph |> prune_subtree(id);
  let added = graph |> add_module(id);

  (removed |> List.filter(id => !List.mem(id, added)), added);
};
