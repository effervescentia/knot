use super::{base::Base, IsVerbose, Parsed};
use crate::{graph::Graph, report};
use lang::ModuleId;
use std::ops::{Deref, DerefMut};

#[derive(Clone)]
pub struct Linked(pub Base<()>, pub Graph);

impl Linked {
    pub fn new(state: Parsed, graph: Graph) -> Self {
        Self(state.0, graph)
    }

    pub fn iter_graph(&self) -> impl Iterator<Item = ModuleId> + '_ {
        self.1.iter()
    }

    pub fn report(&self) {
        if self.is_verbose() {
            eprintln!("\u{1f517} linked all modules and libraries");
        }
    }
}

impl Deref for Linked {
    type Target = Base<()>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Linked {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl report::Enrich for Linked {
    fn enrich(&self, root_dir: String, failure: report::Failure) -> report::Report {
        self.0.enrich(root_dir, failure)
    }
}
