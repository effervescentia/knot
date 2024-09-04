use crate::Link;
use lang::{CanonicalId, Identify};
use std::{
    collections::HashMap,
    fmt::{Debug, Display, Pointer},
};

#[derive(Clone, Debug)]
pub enum Ast<Meta> {
    Program(lang::ast::meta::Program<Meta>),
    Typings(lang::ast::meta::Typings<Meta>),
}

impl<Meta> Ast<Meta> {
    pub fn to_links(&self, link: &Link) -> Vec<Link> {
        let path = link.to_path();

        if let Self::Program(program) = self {
            program
                .imports()
                .iter()
                .map(|x| Link::from_import(&path, x.0.value()))
                .collect::<Vec<_>>()
        } else {
            vec![]
        }
    }
}

impl<Meta> Ast<Meta>
where
    Meta: Clone,
{
    pub fn analyze(
        &self,
        context: &analyze::Context,
    ) -> analyze::Result<(Ast<lang::ast::typed::Meta>, analyze::TypeMap)> {
        match self {
            Self::Program(program) => analyze::analyze(context, program.clone())
                .map(|(typed, types)| (Ast::Program(typed), types)),

            Self::Typings(typings) => analyze::analyze(context, typings.clone())
                .map(|(typed, types)| (Ast::Typings(typed), types)),
        }
    }
}

impl Ast<lang::ast::typed::Meta> {
    pub fn id(&self) -> &CanonicalId {
        match self {
            Self::Program(x) => x.0.id(),
            Self::Typings(x) => x.0.id(),
        }
    }

    pub fn exports(&self) -> HashMap<String, CanonicalId> {
        match self {
            Self::Program(x) => x.exports(),
            Self::Typings(x) => x.exports(),
        }
    }
}

impl<Meta> Display for Ast<Meta>
where
    Meta: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Program(x) => x.fmt(f),
            Self::Typings(x) => x.fmt(f),
        }
    }
}
