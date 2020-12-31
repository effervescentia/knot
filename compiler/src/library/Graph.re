/**
 * directional, acyclic graph
 * can be safely compared through structural equality
 */

type t('a) = {
  mutable nodes: list('a),
  /**
   * these relationships are directional
   * lhs is the parent, rhs is the child
   */
  mutable edges: list(edge_t('a)),
}
and edge_t('a) = ('a, 'a);

let _fold_edges = (fold: ('b, ('a, 'a)) => 'b, acc: 'b, graph: t('a)) =>
  graph.edges |> List.fold_left(fold, acc);

let create = (): t('a) => {nodes: [], edges: []};

let get_nodes = (graph: t('a)): list('a) => graph.nodes;

let has_node = (node: 'a, graph: t('a)): bool =>
  graph.nodes |> List.mem(node);

let is_edge_of = (node: 'a, (parent, child): edge_t('a)): bool =>
  parent == node || child == node;

let get_edges = (node: 'a, graph: t('a)): list(edge_t('a)) =>
  graph
  |> _fold_edges(
       (acc, edge) => is_edge_of(node, edge) ? [edge, ...acc] : acc,
       [],
     );

let get_neighbors = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (parent, child)) =>
         child == node
           ? [parent, ...acc] : parent == node ? [child, ...acc] : acc,
       [],
     );

let get_parents = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (parent, child)) => child == node ? [parent, ...acc] : acc,
       [],
     );

let get_children = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (parent, child)) => parent == node ? [child, ...acc] : acc,
       [],
     );

let get_ancestors = (node: 'a, graph: t('a)): list('a) =>
  graph |> _fold_edges((acc, (parent, child)) => [], []);

let add_node = (node: 'a, graph: t('a)) =>
  graph.nodes = List.incl(node, graph.nodes);

let add_edge = (parent: 'a, child: 'a, graph: t('a)) =>
  graph.edges = List.incl((parent, child), graph.edges);

let remove_node = (node: 'a, graph: t('a)) => {
  graph.nodes = List.excl(node, graph.nodes);
  graph.edges = graph.edges |> List.filter(edge => !is_edge_of(node, edge));
};

let rec remove_subtree = (node: 'a, graph: t('a)) => {
  let children = graph |> get_children(node);

  graph |> remove_node(node);
  children |> List.iter(child => graph |> remove_subtree(child));
};

let find_cycles = (target_node: 'a, graph: t('a)): list(list('a)) => {
  let rec loop = (visited, node) => {
    let neighbors =
      graph |> get_neighbors(node) |> List.filter(n => !List.mem(n, visited));

    neighbors
    |> List.map(n =>
         n == target_node ? [visited] : loop([node, ...visited], n)
       )
    |> List.flatten;
  };

  loop([], target_node);
};

let find_unique_cycles = (nodes: list('a), graph: t('a)): list(list('a)) =>
  nodes
  |> List.map(node =>
       find_cycles(node, graph) |> List.uniq_by(List.compare_members)
     )
  |> List.flatten;

let find_all_unique_cycles = (graph: t('a)): list(list('a)) =>
  graph |> find_unique_cycles(graph.nodes);
