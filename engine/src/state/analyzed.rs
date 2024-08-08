use super::{base::Base, Linked, Module};
use crate::{link::ImportGraph, report, Link};
use std::{
    collections::HashMap,
    ops::{Deref, DerefMut},
};

pub struct Analyzed(pub Base<lang::ast::typed::Meta>, pub ImportGraph);

impl Analyzed {
    pub fn new(state: Linked, modules: HashMap<Link, Module<lang::ast::typed::Meta>>) -> Self {
        Self(state.0.with_modules(modules), state.1)
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
    fn enrich(&self, failure: report::Failure) -> report::Report {
        self.0.enrich(failure)
    }
}
