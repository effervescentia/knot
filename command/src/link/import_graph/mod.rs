mod display;

use bimap::BiMap;
use petgraph::{
    algo::is_cyclic_directed,
    stable_graph::{NodeIndex, StableDiGraph},
    Direction,
};
use std::{collections::HashSet, hash::Hash};

#[derive(Debug, Eq)]
pub struct Cycle(Vec<usize>);

impl Cycle {
    fn canonical(&self) -> Vec<usize> {
        if self.0.is_empty() {
            return vec![];
        }

        let mut canonical = self.0.clone();
        let first_index = canonical
            .iter()
            .enumerate()
            .min_by(|(_, lhs), (_, rhs)| lhs.cmp(rhs))
            .map(|(index, _)| index)
            .unwrap();

        canonical.rotate_left(first_index);

        canonical
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
        self.canonical().hash(state)
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

    fn try_index_of(&self, id: &usize) -> Option<NodeIndex> {
        self.lookup.get_by_left(id).copied()
    }

    fn index_of(&self, id: &usize) -> NodeIndex {
        self.try_index_of(id)
            .expect(format!("node with id {id} not found in the lookup").as_str())
    }

    fn get_node(&self, index: &NodeIndex) -> usize {
        *self
            .lookup
            .get_by_right(index)
            .expect(format!("node with index {index:?} not found in the lookup").as_str())
    }

    pub fn add_node(&mut self, node: usize) -> NodeIndex {
        if let Some(index) = self.try_index_of(&node) {
            return index;
        }

        let index = self.graph.add_node(node);

        self.lookup.insert(node, index);

        index
    }

    pub fn add_edge(&mut self, from: &usize, to: &usize) {
        let from_index = self.index_of(from);
        let to_index = self.index_of(to);

        self.graph.add_edge(from_index, to_index, ());
    }

    pub fn roots<'a>(&'a self) -> impl Iterator<Item = usize> + 'a {
        self.graph
            .node_weights()
            .filter(|x| self.parents(x).count() == 0)
            .copied()
    }

    fn neighbors<'a>(
        &'a self,
        node: &'a usize,
        direction: Direction,
    ) -> impl Iterator<Item = usize> + 'a {
        self.graph
            .neighbors_directed(self.index_of(&node), direction)
            .map(|x| self.get_node(&x))
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
            graph: &ImportGraph,
            visited: &mut HashSet<usize>,
            chain: Vec<usize>,
            node: usize,
        ) -> Vec<Cycle> {
            let parents = graph.parents(&node);

            parents
                .flat_map(|parent| {
                    if Some(&parent) == chain.first() {
                        return vec![Cycle(chain.clone())];
                    }

                    if chain.contains(&parent) {
                        return vec![];
                    }

                    let mut chain = chain.clone();
                    chain.push(parent);

                    visit(graph, visited, chain, parent)
                })
                .collect()
        }

        if visited.contains(&node) {
            return HashSet::new();
        }

        HashSet::from_iter(visit(self, visited, vec![node], node))
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

#[cfg(test)]
mod tests {
    use super::{Cycle, ImportGraph};
    use std::collections::HashSet;

    impl ImportGraph {
        pub fn from_nodes(nodes: Vec<usize>) -> Self {
            let mut graph = Self::new();

            nodes.iter().for_each(|x| {
                graph.add_node(*x);
            });

            graph
        }

        pub fn from_edges(edges: Vec<(usize, usize)>) -> Self {
            let mut graph = Self::new();

            edges.iter().for_each(|(from, to)| {
                graph.add_node(*from);
                graph.add_node(*to);
                graph.add_edge(from, to)
            });

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
        let mut graph = ImportGraph::from_nodes(vec![0, 1]);

        graph.add_edge(&0, &1);

        assert_eq!(graph.graph.edge_count(), 1);
    }

    #[test]
    fn add_existing_node() {
        let mut graph = ImportGraph::from_nodes(vec![0]);

        let index = graph.add_node(0);

        assert_eq!(index.index(), 0);
        assert_eq!(graph.lookup.left_values().collect::<Vec<_>>(), vec![&0]);
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn parents() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (2, 1), (3, 1)]);

        assert_eq!(graph.parents(&1).collect::<Vec<_>>(), vec![3, 2, 0]);
    }

    #[test]
    fn children() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (0, 2), (0, 3)]);

        assert_eq!(graph.children(&0).collect::<Vec<_>>(), vec![3, 2, 1]);
    }

    #[test]
    fn is_cyclic_false() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 3)]);

        assert_eq!(graph.is_cyclic(), false);
    }

    #[test]
    fn is_cyclic_true() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 0)]);

        assert_eq!(graph.is_cyclic(), true);
    }

    #[test]
    fn cycles_with_none() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 0), (4, 5)]);

        assert_eq!(graph.cycles_with(&4), HashSet::new());
    }

    #[test]
    fn cycles_with_one() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 0)]);

        assert_eq!(
            graph.cycles_with(&2),
            HashSet::from_iter(vec![Cycle(vec![2, 1, 0])])
        );
    }

    #[test]
    fn cycles_with_multiple() {
        let graph =
            ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 0), (2, 3), (3, 2), (2, 4), (4, 1)]);

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
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 3)]);

        assert_eq!(graph.cycles(), HashSet::new());
    }

    #[test]
    fn cycles_one() {
        let graph = ImportGraph::from_edges(vec![(0, 1), (1, 2), (2, 0)]);

        assert_eq!(
            graph.cycles(),
            HashSet::from_iter(vec![Cycle(vec![0, 2, 1])])
        );
    }

    #[test]
    fn cycles_multiple() {
        let graph =
            ImportGraph::from_edges(vec![(0, 1), (1, 2), (1, 5), (2, 0), (3, 4), (4, 3), (5, 0)]);

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
