mod display;

use bimap::BiMap;
use kore::invariant;
use lang::ModuleId;
use petgraph::{
    algo::is_cyclic_directed,
    stable_graph::{EdgeIndex, NodeIndex, StableDiGraph},
    visit::{EdgeRef, IntoEdgeReferences, Reversed, Topo, Walker},
    Direction,
};
use std::{collections::HashSet, hash::Hash, iter::empty};

#[derive(Debug, Eq)]
pub struct Cycle(Vec<ModuleId>);

impl Cycle {
    fn canonical(&self) -> Vec<ModuleId> {
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

    pub fn to_vec(&self) -> Vec<ModuleId> {
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

#[derive(Clone, Debug)]
pub struct Graph {
    graph: StableDiGraph<ModuleId, ()>,
    lookup: BiMap<ModuleId, NodeIndex>,
}

impl Graph {
    pub fn new() -> Self {
        Self {
            graph: StableDiGraph::new(),
            lookup: BiMap::new(),
        }
    }

    pub fn size(&self) -> usize {
        self.graph.node_count()
    }

    fn index_of(&self, id: &ModuleId) -> Option<NodeIndex> {
        self.lookup.get_by_left(id).copied()
    }

    fn get_node(&self, index: &NodeIndex) -> ModuleId {
        *self
            .lookup
            .get_by_right(index)
            .unwrap_or_else(|| invariant!("node with index {index:?} not found in the lookup"))
    }

    pub fn upsert_node(&mut self, node: ModuleId) -> NodeIndex {
        if let Some(index) = self.index_of(&node) {
            return index;
        }

        let index = self.graph.add_node(node);

        self.lookup.insert(node, index);

        index
    }

    pub fn remove_node(&mut self, node: &ModuleId) {
        if let Some(index) = self.index_of(node) {
            self.graph.remove_node(index);
            self.lookup.remove_by_left(node);
        }
    }

    pub fn add_edge(&mut self, from: &ModuleId, to: &ModuleId) -> Result<EdgeIndex, ()> {
        if let (Some(from_index), Some(to_index)) = (self.index_of(from), self.index_of(to)) {
            Ok(self.graph.add_edge(from_index, to_index, ()))
        } else {
            Err(())
        }
    }

    pub fn roots(&self) -> impl Iterator<Item = ModuleId> + '_ {
        self.graph
            .node_weights()
            .filter(|x| self.parents(x).count() == 0)
            .copied()
    }

    fn neighbors<'a>(
        &'a self,
        node: &'a ModuleId,
        direction: Direction,
    ) -> Box<dyn Iterator<Item = ModuleId> + 'a> {
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

    pub fn parents<'a>(&'a self, node: &'a ModuleId) -> impl Iterator<Item = ModuleId> + 'a {
        self.neighbors(node, Direction::Incoming)
    }

    pub fn children<'a>(&'a self, node: &'a ModuleId) -> impl Iterator<Item = ModuleId> + 'a {
        self.neighbors(node, Direction::Outgoing)
    }

    pub fn edges(&self) -> impl Iterator<Item = (ModuleId, ModuleId)> + '_ {
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

    fn unvisited_cycles_with(
        &self,
        visited: &mut HashSet<ModuleId>,
        node: ModuleId,
    ) -> HashSet<Cycle> {
        fn visit(
            _visited: &mut HashSet<ModuleId>,
            graph: &Graph,
            chain: &[ModuleId],
            node: ModuleId,
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

    pub fn cycles_with(&self, node: &ModuleId) -> HashSet<Cycle> {
        self.unvisited_cycles_with(&mut HashSet::new(), *node)
    }

    pub fn cycles(&self) -> HashSet<Cycle> {
        let mut visited = HashSet::new();

        self.graph
            .node_weights()
            .flat_map(|x| self.unvisited_cycles_with(&mut visited, *x))
            .collect()
    }

    pub fn iter(&self) -> impl Iterator<Item = ModuleId> + '_ {
        let graph = Reversed(&self.graph);
        let reverse_topological_walker = Topo::new(graph);

        reverse_topological_walker
            .iter(graph)
            .map(|x| self.get_node(&x))
    }
}

impl Default for Graph {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::{Cycle, Graph};
    use lang::ModuleId;
    use std::collections::HashSet;

    #[allow(clippy::multiple_inherent_impl)]
    impl Graph {
        pub fn from_nodes(nodes: &[ModuleId]) -> Self {
            let mut graph = Self::new();

            for x in nodes {
                graph.upsert_node(*x);
            }

            graph
        }

        pub fn from_edges(edges: &[(ModuleId, ModuleId)]) -> Self {
            let mut graph = Self::new();

            for (from, to) in edges {
                graph.upsert_node(*from);
                graph.upsert_node(*to);
                graph.add_edge(from, to).ok();
            }

            graph
        }
    }

    #[test]
    fn add_new_node() {
        let mut graph = Graph::new();

        let index = graph.upsert_node(ModuleId(0));

        assert_eq!(index.index(), 0);
        assert_eq!(
            graph.lookup.left_values().collect::<Vec<_>>(),
            vec![&ModuleId(0)]
        );
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn add_edge() {
        let mut graph = Graph::from_nodes(&[ModuleId(0), ModuleId(1)]);

        graph.add_edge(&ModuleId(0), &ModuleId(1)).ok();

        assert_eq!(graph.graph.edge_count(), 1);
    }

    #[test]
    fn add_existing_node() {
        let mut graph = Graph::from_nodes(&[ModuleId(0)]);

        let index = graph.upsert_node(ModuleId(0));

        assert_eq!(index.index(), 0);
        assert_eq!(
            graph.lookup.left_values().collect::<Vec<_>>(),
            vec![&ModuleId(0)]
        );
        assert_eq!(graph.graph.node_count(), 1);
    }

    #[test]
    fn parents() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(2), ModuleId(1)),
            (ModuleId(3), ModuleId(1)),
        ]);

        assert_eq!(
            graph.parents(&ModuleId(1)).collect::<Vec<_>>(),
            vec![ModuleId(3), ModuleId(2), ModuleId(0)]
        );
    }

    #[test]
    fn children() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(0), ModuleId(2)),
            (ModuleId(0), ModuleId(3)),
        ]);

        assert_eq!(
            graph.children(&ModuleId(0)).collect::<Vec<_>>(),
            vec![ModuleId(3), ModuleId(2), ModuleId(1)]
        );
    }

    #[test]
    fn is_cyclic_false() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(3)),
        ]);

        assert!(!graph.is_cyclic());
    }

    #[test]
    fn is_cyclic_true() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(0)),
        ]);

        assert!(graph.is_cyclic());
    }

    #[test]
    fn cycles_with_none() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(0)),
            (ModuleId(4), ModuleId(5)),
        ]);

        assert_eq!(graph.cycles_with(&ModuleId(4)), HashSet::new());
    }

    #[test]
    fn cycles_with_one() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(0)),
        ]);

        assert_eq!(
            graph.cycles_with(&ModuleId(2)),
            HashSet::from([Cycle(vec![ModuleId(2), ModuleId(1), ModuleId(0)])])
        );
    }

    #[test]
    fn cycles_with_multiple() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(0)),
            (ModuleId(2), ModuleId(3)),
            (ModuleId(3), ModuleId(2)),
            (ModuleId(2), ModuleId(4)),
            (ModuleId(4), ModuleId(1)),
        ]);

        assert_eq!(
            graph.cycles_with(&ModuleId(2)),
            HashSet::from([
                Cycle(vec![ModuleId(2), ModuleId(3)]),
                Cycle(vec![ModuleId(2), ModuleId(1), ModuleId(4)]),
                Cycle(vec![ModuleId(2), ModuleId(1), ModuleId(0)])
            ])
        );
    }

    #[test]
    fn cycles_none() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(3)),
        ]);

        assert_eq!(graph.cycles(), HashSet::new());
    }

    #[test]
    fn cycles_one() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(2), ModuleId(0)),
        ]);

        assert_eq!(
            graph.cycles(),
            HashSet::from([Cycle(vec![ModuleId(0), ModuleId(2), ModuleId(1)])])
        );
    }

    #[test]
    fn cycles_multiple() {
        let graph = Graph::from_edges(&[
            (ModuleId(0), ModuleId(1)),
            (ModuleId(1), ModuleId(2)),
            (ModuleId(1), ModuleId(5)),
            (ModuleId(2), ModuleId(0)),
            (ModuleId(3), ModuleId(4)),
            (ModuleId(4), ModuleId(3)),
            (ModuleId(5), ModuleId(0)),
        ]);

        assert_eq!(
            graph.cycles(),
            HashSet::from([
                Cycle(vec![ModuleId(3), ModuleId(4)]),
                Cycle(vec![ModuleId(0), ModuleId(5), ModuleId(1)]),
                Cycle(vec![ModuleId(0), ModuleId(2), ModuleId(1)])
            ])
        );
    }
}
