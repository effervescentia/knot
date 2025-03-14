use super::{base::Base, IsVerbose, Linked, Module};
use crate::{report, Link};
use kore::graph::Graph;
use lang::ModuleId;
use std::{
    collections::HashMap,
    ops::{Deref, DerefMut},
};

#[derive(Clone)]
pub struct Analyzed(pub Base<lang::ast::typed::Meta>, pub Graph<ModuleId>);

impl Analyzed {
    pub fn new(state: Linked, modules: HashMap<Link, Module<lang::ast::typed::Meta>>) -> Self {
        let verbose = state.is_verbose();
        Self(state.0.with_modules(modules, verbose), state.1)
    }

    pub fn report(&self) {
        if self.is_verbose() {
            eprintln!("\u{2705} analysis complete");
        }
    }
}

impl Deref for Analyzed {
    type Target = Base<lang::ast::typed::Meta>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Analyzed {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl report::Enrich for Analyzed {
    fn enrich(&self, root_dir: String, failure: report::Failure) -> report::Report {
        self.0.enrich(root_dir, failure)
    }
}
