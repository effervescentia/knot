use crate::graph::{Cycle, Graph};
use bimap::BiMap;
use kore::Incrementor;
use lang::{ast, ModuleId};
use std::{
    collections::{HashMap, HashSet},
    fmt::{Display, Formatter},
    path::{Path, PathBuf},
};

#[derive(Debug, Default)]
pub struct Modules {
    incrementor: Incrementor,

    registry: BiMap<ModuleId, PathBuf>,

    dependencies: Graph,

    map: HashMap<ModuleId, Module>,
}

impl Modules {
    pub fn register<T>(&mut self, path: T) -> ModuleId
    where
        T: AsRef<Path>,
    {
        self.get_id_by_path(&path).unwrap_or_else(|| {
            let id = ModuleId(self.incrementor.increment());

            self.registry.insert(id, path.as_ref().to_path_buf());
            self.dependencies.upsert_node(id);

            id
        })
    }

    pub fn add_dependency(&mut self, from: &ModuleId, to: &ModuleId) {
        self.dependencies.add_edge(from, to).unwrap();
    }

    pub fn get_dependents(&self, id: &ModuleId) -> HashSet<ModuleId> {
        self.dependencies.parents(id).collect()
    }

    pub fn insert(&mut self, id: ModuleId, module: Module) {
        self.map.insert(id, module);
    }

    pub fn get_id_by_path<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.registry.get_by_right(path.as_ref()).copied()
    }

    pub fn get_by_id(&self, id: &ModuleId) -> Option<&Module> {
        self.map.get(id)
    }

    pub fn get_by_path<T>(&self, path: T) -> Option<&Module>
    where
        T: AsRef<Path>,
    {
        self.get_id_by_path(path).and_then(|id| self.get_by_id(&id))
    }

    pub fn get_dependency_cycles(&self) -> Option<HashSet<Cycle>> {
        if !self.dependencies.is_cyclic() {
            return None;
        }

        Some(self.dependencies.cycles())
    }

    #[cfg(test)]
    pub fn dependencies(&self) -> &Graph {
        &self.dependencies
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    /// all available data can be used
    Active,

    /// must be re-analyzed before the typed AST can be used
    Stale,

    /// must be re-loaded and processed before it can be used
    Tainted,

    /// will be purged on the next incremental execution
    Evicted,
}

#[derive(Debug, PartialEq)]
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

#[derive(Debug)]
pub struct Module {
    pub id: ModuleId,
    pub path: PathBuf,
    pub text: String,
    pub raw: Ast<()>,
    pub typed: Option<Ast<ast::typed::Meta>>,
    pub status: Status,
}
