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

let rec add_dependency = (source: Namespace.t, target: Namespace.t, graph: t) =>
  graph.imports |> Graph.add_edge(source, target);

let rec add_module = (~added=ref([]), id: Namespace.t, graph: t) => {
  Graph.add_node(id, graph.imports);

  added := added^ |> List.incl(id);

  graph.get_imports(id)
  |> List.iter(child_id => {
       if (!(graph.imports |> Graph.has_node(child_id))) {
         add_module(~added, child_id, graph) |> ignore;
       };

       graph |> add_dependency(id, child_id);
     });

  added^;
};

let init = (entry: Namespace.t) => add_module(entry) % ignore;

let remove_module = (id: Namespace.t, graph: t) => {
  graph.imports |> Graph.remove_node(id);
  graph.imports |> Graph.remove_edges_from(id);
};

let prune_subtree = (node: 'a, graph: t) => {
  let removed = ref([]);

  let rec loop = target => {
    let children = graph.imports |> Graph.get_children(target);

    graph |> remove_module(target);

    removed := removed^ |> List.incl(target);

    children
    |> List.iter(child =>
         if (graph.imports |> Graph.get_parents(child) |> List.is_empty) {
           loop(child);
         }
       );
  };

  loop(node);

  removed^;
};

let get_modules = (graph: t) => graph.imports |> Graph.get_nodes;

let get_dependents = (entry: Namespace.t, graph: t) =>
  graph.imports |> Graph.get_ancestors(entry);

let get_dependencies = (entry: Namespace.t, graph: t) =>
  graph.imports |> Graph.get_descendants(entry);

let has_module = (entry: Namespace.t, graph: t) =>
  graph.imports |> Graph.has_node(entry);

let find_cycles = (graph: t) => graph.imports |> Graph.find_all_unique_cycles;

let find_missing = (graph: t) =>
  graph.imports
  |> Graph.get_edges
  |> List.filter_map(
       snd
       % (child => Graph.has_node(child, graph.imports) ? None : Some(child)),
     )
  |> List.uniq_by((==));

/**
 compare two ImportGraphs for equality
 */
let compare = (l: t, r: t): bool =>
  l.imports == r.imports && l.get_imports === r.get_imports;

/**
 remove all imports from the graph
 */
let clear = ({imports}: t) => Graph.clear(imports);

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, graph: t) => Graph.pp(Namespace.pp, ppf, graph.imports);
