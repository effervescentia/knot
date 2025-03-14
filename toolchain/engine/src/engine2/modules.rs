use bimap::BiMap;
use kore::{
    graph::{Cycle, Graph},
    Incrementor,
};
use lang::{ast, CanonicalId, Identify, ModuleId, Namespace};
use std::{
    collections::{HashMap, HashSet},
    fmt::{Display, Formatter},
    path::{Path, PathBuf},
};

#[derive(Debug, Default)]
pub struct Modules {
    incrementor: Incrementor,

    registry: BiMap<ModuleId, PathBuf>,

    dependencies: Graph<ModuleId>,

    data: HashMap<ModuleId, Module>,
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
        self.data.insert(id, module);
    }

    pub fn get_id_by_path<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.registry.get_by_right(path.as_ref()).copied()
    }

    pub fn get_by_id(&self, id: &ModuleId) -> Option<&Module> {
        self.data.get(id)
    }

    pub fn get_by_path<T>(&self, path: T) -> Option<&Module>
    where
        T: AsRef<Path>,
    {
        self.get_id_by_path(path).and_then(|id| self.get_by_id(&id))
    }

    pub fn get_dependency_cycles(&self) -> Option<HashSet<Cycle<ModuleId>>> {
        if !self.dependencies.is_cyclic() {
            return None;
        }

        Some(self.dependencies.cycles())
    }

    pub fn iter(&self) -> impl Iterator<Item = &Module> + '_ {
        self.dependencies
            .iter()
            .filter_map(|id| self.get_by_id(&id))
    }

    #[cfg(test)]
    pub const fn dependencies(&self) -> &Graph<ModuleId> {
        &self.dependencies
    }

    #[cfg(test)]
    pub fn insert_mock<T, U>(
        &mut self,
        path: T,
        text: U,
        ast: ast::raw::Program,
        status: Status,
    ) -> ModuleId
    where
        T: AsRef<Path>,
        U: AsRef<str>,
    {
        let id = self.register(&path);

        self.insert(
            id,
            Module {
                id,
                path: path.as_ref().to_path_buf(),
                namespace: Namespace::from_internal_path(path),
                text: text.as_ref().to_owned(),
                raw: Ast::Program(ast),
                typed: None,
                status,
            },
        );

        id
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

#[derive(Debug)]
pub struct Module {
    pub id: ModuleId,
    pub path: PathBuf,
    pub namespace: Namespace,
    pub text: String,
    pub raw: Ast<()>,
    pub typed: Option<Ast<ast::typed::Meta>>,
    pub status: Status,
}

impl Module {
    pub fn raw<T>(id: ModuleId, path: T, text: String, ast: ast::raw::Program) -> Self
    where
        T: AsRef<Path>,
    {
        Self {
            id,
            path: path.as_ref().to_path_buf(),
            namespace: Namespace::from_internal_path(path),
            text,
            raw: Ast::Program(ast),
            typed: None,
            status: Status::Active,
        }
    }

    pub fn typed(&self, typed: Ast<ast::typed::Meta>) -> Self {
        Self {
            id: self.id,
            path: self.path.clone(),
            namespace: self.namespace.clone(),
            text: self.text.clone(),
            raw: self.raw.clone(),
            typed: Some(typed),
            status: self.status,
        }
    }
}
