mod display;

use crate::invariant;
use bimap::BiMap;
use petgraph::{
    algo::is_cyclic_directed,
    stable_graph::{EdgeIndex, NodeIndex, StableDiGraph},
    visit::{EdgeRef, IntoEdgeReferences, Reversed, Topo, Walker},
    Direction,
};
use std::{collections::HashSet, hash::Hash, iter::empty};

#[derive(Debug, Eq)]
pub struct Cycle<Node>(Vec<Node>)
where
    Node: Clone + Eq + Ord;

impl<Node> Cycle<Node>
where
    Node: Clone + Ord,
{
    fn canonical(&self) -> Vec<Node> {
        self.0
            .iter()
            .enumerate()
            .min_by(|(_, lhs), (_, rhs)| lhs.cmp(rhs))
            .map(|(index, _)| {
                let mut canonical = self.0.clone();
                canonical.rotate_left(index);

                canonical
            })
            .unwrap_or_default()
    }

    pub fn to_vec(&self) -> Vec<Node> {
        self.canonical()
    }
}

impl<Node> PartialEq for Cycle<Node>
where
    Node: Clone + Ord + PartialEq,
{
    fn eq(&self, other: &Self) -> bool {
        self.canonical() == other.canonical()
    }
}

impl<Node> Hash for Cycle<Node>
where
    Node: Clone + Hash + Ord,
{
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.canonical().hash(state);
    }
}

#[derive(Clone, Debug)]
pub struct Graph<Node>
where
    Node: Copy + Eq + Hash,
{
    graph: StableDiGraph<Node, ()>,
    lookup: BiMap<Node, NodeIndex>,
}

impl<Node> Graph<Node>
where
    Node: Copy + Eq + Hash,
{
    pub fn new() -> Self {
        Self {
            graph: StableDiGraph::new(),
            lookup: BiMap::new(),
        }
    }

    pub fn size(&self) -> usize {
        self.graph.node_count()
    }

    fn index_of(&self, node: &Node) -> Option<NodeIndex> {
        self.lookup.get_by_left(node).copied()
    }

    fn get_node(&self, index: &NodeIndex) -> Node {
        *self
            .lookup
            .get_by_right(index)
            .unwrap_or_else(|| invariant!("node with index {index:?} not found in the lookup"))
    }

    pub fn upsert_node(&mut self, node: Node) -> NodeIndex {
        if let Some(index) = self.index_of(&node) {
            return index;
        }

        let index = self.graph.add_node(node);

        self.lookup.insert(node, index);

        index
    }

    pub fn remove_node(&mut self, node: &Node) {
        if let Some(index) = self.index_of(node) {
            self.graph.remove_node(index);
            self.lookup.remove_by_left(node);
        }
    }

    pub fn add_edge(&mut self, from: &Node, to: &Node) -> Option<EdgeIndex> {
        if let (Some(from_index), Some(to_index)) = (self.index_of(from), self.index_of(to)) {
            Some(self.graph.add_edge(from_index, to_index, ()))
        } else {
            None
        }
    }

    pub fn roots(&self) -> impl Iterator<Item = Node> + '_ {
        self.graph
            .node_weights()
            .filter(|x| self.parents(x).count() == 0)
            .copied()
    }

    fn neighbors<'a>(
        &'a self,
        node: &'a Node,
        direction: Direction,
    ) -> Box<dyn Iterator<Item = Node> + 'a> {
        if let Some(node_index) = self.index_of(node) {
            Box::new(
                self.graph
                    .neighbors_directed(node_index, direction)
                    .map(|x| self.get_node(&x)),
            )
        } else {
            Box::new(empty())
        }
    }

    pub fn parents<'a>(&'a self, node: &'a Node) -> impl Iterator<Item = Node> + 'a {
        self.neighbors(node, Direction::Incoming)
    }

    pub fn children<'a>(&'a self, node: &'a Node) -> impl Iterator<Item = Node> + 'a {
        self.neighbors(node, Direction::Outgoing)
    }

    pub fn edges(&self) -> impl Iterator<Item = (Node, Node)> + '_ {
        self.graph.edge_references().filter_map(|x| {
            Some((
                *self.lookup.get_by_right(&x.source())?,
                *self.lookup.get_by_right(&x.target())?,
            ))
        })
    }

    pub fn is_cyclic(&self) -> bool {
        is_cyclic_directed(&self.graph)
    }

    fn unvisited_cycles_with(&self, visited: &mut HashSet<Node>, node: Node) -> HashSet<Cycle<Node>>
    where
        Node: Ord,
    {
        fn visit<U>(
            _visited: &mut HashSet<U>,
            graph: &Graph<U>,
            chain: &[U],
            node: U,
        ) -> Vec<Cycle<U>>
        where
            U: Copy + Eq + Hash + Ord,
        {
            let parents = graph.parents(&node);

            parents
                .flat_map(|parent| {
                    if Some(&parent) == chain.first() {
                        return vec![Cycle(chain.to_vec())];
                    }

                    if chain.contains(&parent) {
                        return vec![];
                    }

                    visit(_visited, graph, &[chain, &[parent]].concat(), parent)
                })
                .collect()
        }

        if visited.contains(&node) {
            return HashSet::new();
        }

        HashSet::from_iter(visit(visited, self, &[node], node))
    }

    pub fn cycles_with(&self, node: &Node) -> HashSet<Cycle<Node>>
    where
        Node: Ord,
    {
        self.unvisited_cycles_with(&mut HashSet::new(), *node)
    }

    pub fn cycles(&self) -> HashSet<Cycle<Node>>
    where
        Node: Ord,
    {
        let mut visited = HashSet::new();

        self.graph
            .node_weights()
            .flat_map(|x| self.unvisited_cycles_with(&mut visited, *x))
            .collect()
    }

    pub fn iter(&self) -> impl Iterator<Item = Node> + '_ {
        let graph = Reversed(&self.graph);
        let reverse_topological_walker = Topo::new(graph);

        reverse_topological_walker
            .iter(graph)
            .map(|x| self.get_node(&x))
    }
}

impl<Node> Default for Graph<Node>
where
    Node: Copy + Eq + Hash,
{
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::{Cycle, Graph};
    use std::collections::HashSet;

    #[allow(clippy::multiple_inherent_impl)]
    impl Graph<usize> {
        pub fn from_nodes(nodes: &[usize]) -> Self {
            let mut graph = Self::new();

            for x in nodes {
                graph.upsert_node(*x);
            }

            graph
        }

        pub fn from_edges(edges: &[(usize, usize)]) -> Self {
            let mut graph = Self::new();

            for (from, to) in edges {
                graph.upsert_node(*from);
                graph.upsert_node(*to);
                graph.add_edge(from, to);
            }

            graph
        }
    }

    #[test]
    fn add_new_node() {
        let mut graph = Graph::new();

        let index = graph.upsert_node(0);

        assert_eq!(index.index(), 0);
        assert_eq!(graph.lookup.left_values().collect::<Vec<_>>(), vec![&0]);
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn add_edge() {
        let mut graph = Graph::from_nodes(&[0, 1]);

        graph.add_edge(&0, &1);

        assert_eq!(graph.graph.edge_count(), 1);
    }

    #[test]
    fn add_existing_node() {
        let mut graph = Graph::from_nodes(&[0]);

        let index = graph.upsert_node(0);

        assert_eq!(index.index(), 0);
        assert_eq!(graph.lookup.left_values().collect::<Vec<_>>(), vec![&0]);
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn parents() {
        let graph = Graph::from_edges(&[(0, 1), (2, 1), (3, 1)]);

        assert_eq!(graph.parents(&1).collect::<Vec<_>>(), vec![3, 2, 0]);
    }

    #[test]
    fn children() {
        let graph = Graph::from_edges(&[(0, 1), (0, 2), (0, 3)]);

        assert_eq!(graph.children(&0).collect::<Vec<_>>(), vec![3, 2, 1]);
    }

    #[test]
    fn is_cyclic_false() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 3)]);

        assert!(!graph.is_cyclic());
    }

    #[test]
    fn is_cyclic_true() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert!(graph.is_cyclic());
    }

    #[test]
    fn cycles_with_none() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 0), (4, 5)]);

        assert_eq!(graph.cycles_with(&4), HashSet::new());
    }

    #[test]
    fn cycles_with_one() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert_eq!(graph.cycles_with(&2), HashSet::from([Cycle(vec![2, 1, 0])]));
    }

    #[test]
    fn cycles_with_multiple() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 0), (2, 3), (3, 2), (2, 4), (4, 1)]);

        assert_eq!(
            graph.cycles_with(&2),
            HashSet::from([
                Cycle(vec![2, 3]),
                Cycle(vec![2, 1, 4]),
                Cycle(vec![2, 1, 0])
            ])
        );
    }

    #[test]
    fn cycles_none() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 3)]);

        assert_eq!(graph.cycles(), HashSet::new());
    }

    #[test]
    fn cycles_one() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert_eq!(graph.cycles(), HashSet::from([Cycle(vec![0, 2, 1])]));
    }

    #[test]
    fn cycles_multiple() {
        let graph = Graph::from_edges(&[(0, 1), (1, 2), (1, 5), (2, 0), (3, 4), (4, 3), (5, 0)]);

        assert_eq!(
            graph.cycles(),
            HashSet::from([
                Cycle(vec![3, 4]),
                Cycle(vec![0, 5, 1]),
                Cycle(vec![0, 2, 1])
            ])
        );
    }
}
