use std::{collections::HashSet, hash::Hash};

pub struct Graph<T> {
    nodes: HashSet<T>,
    edges: HashSet<(T, T)>,
}

impl<T> Graph<T> {
    pub fn new() -> Self {
        Graph {
            nodes: HashSet::new(),
            edges: HashSet::new(),
        }
    }
}

impl<T> Graph<T>
where
    T: Eq + Hash + PartialEq,
{
    pub fn add_node(&mut self, node: T) {
        self.nodes.insert(node);
    }

    pub fn add_edge(&mut self, from: T, to: T) {
        self.edges.insert((from, to));
    }
}
