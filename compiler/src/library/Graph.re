/**
 * Directional graph with support for cycle detection.
 * Can be safely compared through structural equality.
 */
open Infix;

exception InvalidEdge;

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

let _is_edge_of = (node: 'a, (parent, child): edge_t('a)): bool =>
  parent == node || child == node;
let _is_edge_to = (node: 'a, (_, child): edge_t('a)): bool => child == node;
let _is_edge_from = (node: 'a, (parent, _): edge_t('a)): bool =>
  parent == node;

let empty = (): t('a) => {nodes: [], edges: []};

let create = (nodes: list('a), edges: list(edge_t('a))): t('a) => {
  nodes,
  edges,
};

let get_nodes = (graph: t('a)): list('a) => graph.nodes;

let has_node = (node: 'a, graph: t('a)): bool =>
  graph.nodes |> List.mem(node);

let get_edges = (graph: t('a)): list(edge_t('a)) => graph.edges;

let get_edges_of = (node: 'a, graph: t('a)): list(edge_t('a)) =>
  graph
  |> _fold_edges(
       (acc, edge) => _is_edge_of(node, edge) ? [edge, ...acc] : acc,
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
       (acc, (parent, _) as edge) =>
         _is_edge_to(node, edge) ? [parent, ...acc] : acc,
       [],
     );

let get_children = (node: 'a, graph: t('a)): list('a) =>
  graph
  |> _fold_edges(
       (acc, (_, child) as edge) =>
         _is_edge_from(node, edge) ? [child, ...acc] : acc,
       [],
     );

let is_root_node = (node: 'a, graph: t('a)): bool =>
  graph |> get_parents(node) |> List.length |> (==)(0);

let find_roots = (graph: t('a)): list('a) =>
  graph.nodes |> List.filter(node => is_root_node(node, graph));

let add_node = (node: 'a, graph: t('a)) =>
  graph.nodes = List.incl(node, graph.nodes);

let add_edge = (parent: 'a, child: 'a, graph: t('a)) => {
  if (!List.mem(parent, graph.nodes) || !List.mem(child, graph.nodes)) {
    raise(InvalidEdge);
  };

  graph.edges = List.incl((parent, child), graph.edges);
};

let remove_node = (node: 'a, graph: t('a)) =>
  graph.nodes = List.excl(node, graph.nodes);

let remove_edges_to = (node: 'a, graph: t('a)) =>
  graph.edges = graph.edges |> List.filter(edge => !_is_edge_to(node, edge));

let remove_edges_from = (node: 'a, graph: t('a)) =>
  graph.edges =
    graph.edges |> List.filter(edge => !_is_edge_from(node, edge));

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

let find_unique_cycles = (nodes: list('a), graph: t('a)): list(list('a)) =>
  nodes
  |> List.map(node => find_cycles(node, graph))
  |> List.flatten
  |> List.uniq_by(List.compare_members);

let find_all_unique_cycles = (graph: t('a)): list(list('a)) =>
  graph |> find_unique_cycles(graph.nodes);

let is_acyclic = (graph: t('a)): bool =>
  find_all_unique_cycles(graph) |> List.length |> (==)(0);

let _merge_trees =
    (depth: int, subtrees: list((int, list(string)))): list(string) => {
  List.repeat(depth, ())
  |> List.mapi((index, _) => {
       subtrees
       |> List.map(((width, rows)) =>
            (List.length(rows) > index ? List.nth(rows, index) : "")
            |> Print.fmt("%-*s", width)
          )
       |> String.join(~separator="")
     });
};

let _tree_of_rows = (rows: list(string)): (int, list(string)) => {
  let width =
    rows |> List.map(String.length) |> Int.max_of |> (+)(1) |> max(2);

  (width, rows |> List.map(Print.fmt("%-*s", width)));
};

let rec _print_subtree =
        (
          ~ancestors=[],
          node: 'a,
          visited: ref(list('a)),
          print_node: 'a => string,
          graph: t('a),
        )
        : (int, list(string)) => {
  let children = graph |> get_children(node);

  let subtrees =
    List.mem(node, visited^)
      ? List.length(children) == 0 ? [] : [_tree_of_rows(["[…]"])]
      : children
        |> {
          visited := [node, ...visited^];

          List.map(child =>
            List.mem(child, ancestors)
              ? _tree_of_rows([print_node(child), "|", "[cycle]"])
              : _print_subtree(
                  ~ancestors=[node, ...ancestors],
                  child,
                  visited,
                  print_node,
                  graph,
                )
          );
        };
  let has_multiple_subtrees = List.length(subtrees) > 1;
  let depth = subtrees |> List.map(snd % List.length) |> Int.max_of;

  let root_row = print_node(node);
  let pipe_row =
    subtrees
    |> List.map(fst % (width => Print.fmt("%-*s", width, "|")))
    |> String.join(~separator="");
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
        |> String.join(~separator=""),
      ]
      : [];

  let child_rows = _merge_trees(depth, subtrees);

  let rows = [root_row] @ divider_rows @ [pipe_row] @ child_rows;

  _tree_of_rows(rows);
};

/**
 * visited nodes can be printed multiple times, but their subtrees will only be printed once
 * can handle cyclic graphs
 */
let to_string = (print_node: 'a => string, graph: t('a)) => {
  let roots = find_roots(graph);
  let visited: ref(list('a)) = ref([]);

  let printed =
    roots
    |> List.map(root => _print_subtree(root, visited, print_node, graph));
  let depth = printed |> List.map(snd % List.length) |> Int.max_of;

  _merge_trees(depth, printed) |> String.join(~separator="\n");
};
