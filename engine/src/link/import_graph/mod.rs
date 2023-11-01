mod display;

use bimap::BiMap;
use kore::invariant;
use petgraph::{
    algo::is_cyclic_directed,
    stable_graph::{EdgeIndex, NodeIndex, StableDiGraph},
    Direction,
};
use std::{collections::HashSet, hash::Hash, iter::empty};

#[derive(Debug, Eq)]
pub struct Cycle(Vec<usize>);

impl Cycle {
    fn canonical(&self) -> Vec<usize> {
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

    pub fn to_vec(&self) -> Vec<usize> {
        self.canonical()
    }
}

impl PartialEq for Cycle {
    fn eq(&self, other: &Self) -> bool {
        self.canonical() == other.canonical()
    }
}

impl Hash for Cycle {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.canonical().hash(state);
    }
}

pub struct ImportGraph {
    graph: StableDiGraph<usize, ()>,
    lookup: BiMap<usize, NodeIndex>,
}

impl ImportGraph {
    pub fn new() -> Self {
        Self {
            graph: StableDiGraph::new(),
            lookup: BiMap::new(),
        }
    }

    fn index_of(&self, id: &usize) -> Option<NodeIndex> {
        self.lookup.get_by_left(id).copied()
    }

    fn get_node(&self, index: &NodeIndex) -> usize {
        *self
            .lookup
            .get_by_right(index)
            .unwrap_or_else(|| invariant!("node with index {index:?} not found in the lookup"))
    }

    pub fn add_node(&mut self, node: usize) -> NodeIndex {
        if let Some(index) = self.index_of(&node) {
            return index;
        }

        let index = self.graph.add_node(node);

        self.lookup.insert(node, index);

        index
    }

    pub fn add_edge(&mut self, from: &usize, to: &usize) -> Result<EdgeIndex, ()> {
        if let (Some(from_index), Some(to_index)) = (self.index_of(from), self.index_of(to)) {
            Ok(self.graph.add_edge(from_index, to_index, ()))
        } else {
            Err(())
        }
    }

    pub fn roots(&self) -> impl Iterator<Item = usize> + '_ {
        self.graph
            .node_weights()
            .filter(|x| self.parents(x).count() == 0)
            .copied()
    }

    fn neighbors<'a>(
        &'a self,
        node: &'a usize,
        direction: Direction,
    ) -> Box<dyn Iterator<Item = usize> + 'a> {
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

    pub fn parents<'a>(&'a self, node: &'a usize) -> impl Iterator<Item = usize> + 'a {
        self.neighbors(node, Direction::Incoming)
    }

    pub fn children<'a>(&'a self, node: &'a usize) -> impl Iterator<Item = usize> + 'a {
        self.neighbors(node, Direction::Outgoing)
    }

    pub fn is_cyclic(&self) -> bool {
        is_cyclic_directed(&self.graph)
    }

    fn unvisited_cycles_with(&self, visited: &mut HashSet<usize>, node: usize) -> HashSet<Cycle> {
        fn visit(
            _visited: &mut HashSet<usize>,
            graph: &ImportGraph,
            chain: &[usize],
            node: usize,
        ) -> Vec<Cycle> {
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

    pub fn cycles_with(&self, node: &usize) -> HashSet<Cycle> {
        self.unvisited_cycles_with(&mut HashSet::new(), *node)
    }

    pub fn cycles(&self) -> HashSet<Cycle> {
        let mut visited = HashSet::new();

        self.graph
            .node_weights()
            .flat_map(|x| self.unvisited_cycles_with(&mut visited, *x))
            .collect()
    }
}

impl Default for ImportGraph {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::{Cycle, ImportGraph};
    use std::collections::HashSet;

    #[allow(clippy::multiple_inherent_impl)]
    impl ImportGraph {
        pub fn from_nodes(nodes: &[usize]) -> Self {
            let mut graph = Self::new();

            for x in nodes {
                graph.add_node(*x);
            }

            graph
        }

        pub fn from_edges(edges: &[(usize, usize)]) -> Self {
            let mut graph = Self::new();

            for (from, to) in edges {
                graph.add_node(*from);
                graph.add_node(*to);
                graph.add_edge(from, to).ok();
            }

            graph
        }
    }

    #[test]
    fn add_new_node() {
        let mut graph = ImportGraph::new();

        let index = graph.add_node(0);

        assert_eq!(index.index(), 0);
        assert_eq!(graph.lookup.left_values().collect::<Vec<_>>(), vec![&0]);
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn add_edge() {
        let mut graph = ImportGraph::from_nodes(&[0, 1]);

        graph.add_edge(&0, &1).ok();

        assert_eq!(graph.graph.edge_count(), 1);
    }

    #[test]
    fn add_existing_node() {
        let mut graph = ImportGraph::from_nodes(&[0]);

        let index = graph.add_node(0);

        assert_eq!(index.index(), 0);
        assert_eq!(graph.lookup.left_values().collect::<Vec<_>>(), vec![&0]);
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn parents() {
        let graph = ImportGraph::from_edges(&[(0, 1), (2, 1), (3, 1)]);

        assert_eq!(graph.parents(&1).collect::<Vec<_>>(), vec![3, 2, 0]);
    }

    #[test]
    fn children() {
        let graph = ImportGraph::from_edges(&[(0, 1), (0, 2), (0, 3)]);

        assert_eq!(graph.children(&0).collect::<Vec<_>>(), vec![3, 2, 1]);
    }

    #[test]
    fn is_cyclic_false() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 3)]);

        assert!(!graph.is_cyclic());
    }

    #[test]
    fn is_cyclic_true() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert!(graph.is_cyclic());
    }

    #[test]
    fn cycles_with_none() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 0), (4, 5)]);

        assert_eq!(graph.cycles_with(&4), HashSet::new());
    }

    #[test]
    fn cycles_with_one() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert_eq!(
            graph.cycles_with(&2),
            HashSet::from_iter(vec![Cycle(vec![2, 1, 0])])
        );
    }

    #[test]
    fn cycles_with_multiple() {
        let graph =
            ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 0), (2, 3), (3, 2), (2, 4), (4, 1)]);

        assert_eq!(
            graph.cycles_with(&2),
            HashSet::from_iter(vec![
                Cycle(vec![2, 3]),
                Cycle(vec![2, 1, 4]),
                Cycle(vec![2, 1, 0])
            ])
        );
    }

    #[test]
    fn cycles_none() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 3)]);

        assert_eq!(graph.cycles(), HashSet::new());
    }

    #[test]
    fn cycles_one() {
        let graph = ImportGraph::from_edges(&[(0, 1), (1, 2), (2, 0)]);

        assert_eq!(
            graph.cycles(),
            HashSet::from_iter(vec![Cycle(vec![0, 2, 1])])
        );
    }

    #[test]
    fn cycles_multiple() {
        let graph =
            ImportGraph::from_edges(&[(0, 1), (1, 2), (1, 5), (2, 0), (3, 4), (4, 3), (5, 0)]);

        assert_eq!(
            graph.cycles(),
            HashSet::from_iter(vec![
                Cycle(vec![3, 4]),
                Cycle(vec![0, 5, 1]),
                Cycle(vec![0, 2, 1])
            ])
        );
    }
}
