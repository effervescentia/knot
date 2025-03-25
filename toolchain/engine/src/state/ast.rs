use lang::{ast, CanonicalId, Identify};
use std::{
    collections::HashMap,
    fmt::{Display, Formatter},
    path::{Path, PathBuf},
};

#[derive(Clone, Debug, PartialEq)]
pub enum Ast<Meta> {
    Program(ast::meta::Program<Meta>),
}

impl<Meta> Ast<Meta> {
    pub fn get_dependencies<T>(&self, relative_to: T) -> Vec<PathBuf>
    where
        T: AsRef<Path>,
    {
        let Self::Program(program) = self;

        program.get_dependencies(relative_to)
    }

    pub fn analyze(
        &self,
        context: &analyze::Context,
    ) -> analyze::Result<(Ast<lang::ast::typed::Meta>, analyze::TypeMap)>
    where
        Meta: Clone,
    {
        match self {
            Self::Program(program) => analyze::analyze(context, program.clone())
                .map(|(typed, types)| (Ast::Program(typed), types)),
        }
    }
}

impl Ast<ast::typed::Meta> {
    pub fn id(&self) -> &CanonicalId {
        match self {
            Self::Program(x) => x.0.id(),
        }
    }

    pub fn exports(&self) -> HashMap<String, CanonicalId> {
        match self {
            Self::Program(x) => x.exports(),
        }
    }
}

impl<Meta> Display for Ast<Meta>
where
    Meta: Clone,
{
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Program(ast) => ast.fmt(f),
        }
    }
}
