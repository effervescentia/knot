use super::{base::Base, Parsed};
use crate::{link::ImportGraph, report};
use lang::NamespaceId;
use std::ops::{Deref, DerefMut};

#[derive(Clone)]
pub struct Linked(pub Base<()>, pub ImportGraph);

impl Linked {
    pub fn new(state: Parsed, graph: ImportGraph) -> Self {
        Self(state.0, graph)
    }

    pub fn iter_graph(&self) -> impl Iterator<Item = NamespaceId> + '_ {
        self.1.iter()
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
    fn enrich(&self, failure: report::Failure) -> report::Report {
        self.0.enrich(failure)
    }
}
