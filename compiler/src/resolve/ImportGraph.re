open Kore;

type t = Graph.t(m_id);

let rec add_module = (id: m_id, graph: t) => {
  Graph.add_node(id, graph);

  graph
  |> Graph.get_children(id)
  |> List.iter(child_id => {
       if (!(graph |> Graph.has_node(child_id))) {
         add_module(id, graph);
       };

       graph |> Graph.add_edge(id, child_id);
     });
};

let remove_subtree = Graph.remove_subtree;

let refresh_subtree = (id: m_id, graph: t) => {
  graph |> Graph.remove_subtree(id);
  graph |> Graph.add_node(id);
};

let create = (entry: m_id, get_imports: m_id => list(m_id)): t => {
  let graph = Graph.create();

  add_module(entry, graph);

  graph;
};
