use super::base::Base;
use crate::link::ImportGraph;
use std::ops::{Deref, DerefMut};

#[derive(Default)]
pub struct Parsed(pub Base<()>);

impl Parsed {
    pub fn to_import_graph(&self) -> ImportGraph {
        self.internal_modules()
            .fold(ImportGraph::new(), |mut graph, (_, x)| {
                graph.add_node(x.id);
                graph
            })
    }
}

impl Deref for Parsed {
    type Target = Base<()>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Parsed {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
