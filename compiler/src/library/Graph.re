/**
 Directional graph with support for cycle detection.
 Can be safely compared through structural equality.
 */
open Infix;

exception InvalidEdge;

type t('a) = {
  mutable nodes: list('a),
  /**
   these relationships are directional

   lhs is the parent, rhs is the child
   */
  mutable edges: list(edge_t('a)),
}
and edge_t('a) = ('a, 'a);

let _fold_edges = (fold: ('b, ('a, 'a)) => 'b, acc: 'b, graph: t('a)) =>
  graph.edges |> List.fold_left(fold, acc);

let _is_edge_of = (node: 'a, (parent, child): edge_t('a)): bool =>
  parent == node || child == node;

let _is_edge_to = (node: 'a, (_, child): edge_t('a)): bool => child == node;

let _is_edge_from = (node: 'a, (parent, _): edge_t('a)): bool =>
  parent == node;

/* static */

/**
 create a graph from lists of nodes and edges
 */
let create = (nodes: list('a), edges: list(edge_t('a))): t('a) => {
  nodes,
  edges,
};

/**
 create an empty graph
 */
let empty = (): t('a) => create([], []);

/* getters */

let get_nodes = (graph: t('a)): list('a) => graph.nodes;
let get_edges = (graph: t('a)): list(edge_t('a)) => graph.edges;

/* methods */

/**
 check if a [node] exists in the [graph]
 */
let has_node = (node: 'a, graph: t('a)): bool =>
  graph.nodes |> List.mem(node);

/**
 get the edges connected to a [node] in the [graph]
 */
let get_edges_of = (node: 'a, graph: t('a)): list(edge_t('a)) =>
  graph
  |> _fold_edges(
       (acc, edge) => _is_edge_of(node, edge) ? [edge, ...acc] : acc,
       [],
     );

/**
 get a list of nodes in the [graph] that share an edge with a [node]
 */
let get_neighbors = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (parent, child)) =>
         child == node
           ? [parent, ...acc] : parent == node ? [child, ...acc] : acc,
       [],
     );

/**
 get a list of nodes in the [graph] which are a parent of a [node]
 */
let get_parents = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (parent, _) as edge) =>
         _is_edge_to(node, edge) ? [parent, ...acc] : acc,
       [],
     );

/**
 get a list of nodes in the [graph] which are a child of a [node]
 */
let get_children = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (_, child) as edge) =>
         _is_edge_from(node, edge) ? [child, ...acc] : acc,
       [],
     );

/**
 check if a [node] in the [graph] has no parents
 */
let is_root_node = (node: 'a, graph: t('a)): bool =>
  graph |> get_parents(node) |> List.length |> (==)(0);

/**
 get a list of all nodes in the [graph] that have no parents
 */
let find_roots = (graph: t('a)): list('a) =>
  graph.nodes |> List.filter(node => is_root_node(node, graph));

/**
 add a [node] to the [graph]
 */
let add_node = (node: 'a, graph: t('a)) =>
  graph.nodes = List.incl(node, graph.nodes);

/**
 add an edge between a [parent] and a [child] in the [graph]
 */
let add_edge = (parent: 'a, child: 'a, graph: t('a)) => {
  if (!List.mem(parent, graph.nodes) || !List.mem(child, graph.nodes)) {
    raise(InvalidEdge);
  };

  graph.edges = List.incl((parent, child), graph.edges);
};

/**
 combine and de-duplicate the nodes and edges of two graphs
 */
let union = (lhs: t('a), rhs: t('a)): t('a) => {
  nodes:
    lhs.nodes
    @ rhs.nodes
    |> List.fold_left((acc, node) => List.incl(node, acc), []),
  edges:
    lhs.edges
    @ rhs.edges
    |> List.fold_left((acc, edge) => List.incl(edge, acc), []),
};

/**
 remove a [node] from the [graph]
 */
let remove_node = (node: 'a, graph: t('a)) =>
  graph.nodes = List.excl(node, graph.nodes);

/**
 remove edges from the [graph] that connect to [node] as the child
 */
let remove_edges_to = (node: 'a, graph: t('a)) =>
  graph.edges = graph.edges |> List.filter(edge => !_is_edge_to(node, edge));

/**
 remove edges from the [graph] that connect from [node] as the parent
 */
let remove_edges_from = (node: 'a, graph: t('a)) =>
  graph.edges =
    graph.edges |> List.filter(edge => !_is_edge_from(node, edge));

/**
 find cycles in the [graph] that involve a [target_node]
 */
let find_cycles = (target_node: 'a, graph: t('a)): list(list('a)) => {
  let rec loop = (visited, node) => {
    let parents = graph |> get_parents(node);

    parents
    |> List.map(parent =>
         parent == target_node
           ? [visited]
           : List.mem(parent, visited)
               ? [] : loop([parent, ...visited], parent)
       )
    |> List.flatten;
  };

  loop([target_node], target_node);
};

/**
 find unique cycles in the [graph] that involve a list of [nodes]
 */
let find_unique_cycles = (nodes: list('a), graph: t('a)): list(list('a)) =>
  nodes
  |> List.map(node => find_cycles(node, graph))
  |> List.flatten
  |> List.uniq_by(List.compare_members);

/**
 find all unique cycles in the [graph]
 */
let find_all_unique_cycles = (graph: t('a)): list(list('a)) =>
  graph |> find_unique_cycles(graph.nodes);

/**
 check if a [graph] is acyclic
 */
let is_acyclic = (graph: t('a)): bool =>
  find_all_unique_cycles(graph) |> List.length |> (==)(0);

/* pretty printing */

let _tree_of_rows = (rows: list(string)): (int, list(string)) => {
  let width =
    rows |> List.map(String.length) |> Int.max_of |> (+)(1) |> max(2);

  (width, rows |> List.map(Print.fmt("%-*s", width)));
};

let _pad_rows = (rows: list(string)): (int, list(string)) => {
  let width =
    rows |> List.map(String.length) |> Int.max_of |> (+)(1) |> max(2);

  (width, rows |> List.map(Print.fmt("%-*s", width)));
};

let _merge_trees =
    (depth: int, subtrees: list((int, list(string)))): list(string) =>
  List.repeat(depth, ())
  |> List.mapi((index, _) => {
       subtrees
       |> List.map(((width, rows)) =>
            (List.length(rows) > index ? List.nth(rows, index) : "")
            |> Print.fmt("%-*s", width)
          )
       |> String.join
     });

let rec _print_subtree =
        (
          ~ancestors=[],
          ~visited: ref(list('a)),
          pp_node: Fmt.t('a),
          graph: t('a),
          node: 'a,
        )
        : (int, list(string)) => {
  let children = graph |> get_children(node);

  let subtrees =
    switch (List.mem(node, visited^), children) {
    | (true, []) => []
    | (true, _) => [_pad_rows(["[…]"])]
    | (false, _) =>
      visited := [node, ...visited^];

      List.map(
        child =>
          List.mem(child, ancestors)
            ? _tree_of_rows([child |> ~@pp_node, "|", "[cycle]"])
            : _print_subtree(
                ~ancestors=[node, ...ancestors],
                ~visited,
                pp_node,
                graph,
                child,
              ),
        children,
      );
    };
  let has_multiple_subtrees = List.length(subtrees) > 1;
  let depth = subtrees |> List.map(snd % List.length) |> Int.max_of;

  let root_row = node |> ~@pp_node;
  let pipe_row =
    subtrees
    |> List.map(fst % (width => Print.fmt("%-*s", width, "|")))
    |> String.join;
  let divider_rows =
    has_multiple_subtrees
      ? [
        subtrees
        |> List.mapi(index =>
             fst
             % (
               width =>
                 index < List.length(subtrees) - 1
                   ? (index == 0 ? "|" : ",") ++ String.repeat(width - 1, "-")
                   : ","
             )
           )
        |> String.join,
      ]
      : [];
  let child_rows = _merge_trees(depth, subtrees);

  let rows = [root_row] @ divider_rows @ [pipe_row] @ child_rows;

  _pad_rows(rows);
};

/**
 visited nodes can be printed multiple times, but their subtrees will only be printed once

 can handle cyclic graphs
 */
let pp = (pp_node: Fmt.t('a)): Fmt.t(t('a)) =>
  (ppf, graph: t('a)) => {
    let roots = find_roots(graph);
    let visited: ref(list('a)) = ref([]);

    let printed =
      roots
      |> List.map(root => _print_subtree(~visited, pp_node, graph, root));
    let depth = printed |> List.map(snd % List.length) |> Int.max_of;

    Fmt.pf(
      ppf,
      "%s",
      _merge_trees(depth, printed) |> String.join(~separator="\n"),
    );
  };
