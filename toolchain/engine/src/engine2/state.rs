use super::{Context, Operation, Scope};
use crate::graph::Graph;
use bimap::BiMap;
use kore::Incrementor;
use lang::{ast, ModuleId};
use std::{
    collections::{HashMap, HashSet},
    fmt::{Display, Formatter},
    fs,
    path::{Path, PathBuf},
};

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

#[derive(Debug, PartialEq)]
pub struct Module {
    pub id: ModuleId,
    pub path: PathBuf,
    pub text: String,
    pub raw: Ast<()>,
    pub typed: Option<Ast<ast::typed::Meta>>,
    pub status: Status,
}

#[derive(Debug, Default)]
struct Dependencies(Graph);

impl Dependencies {
    pub fn add_dependency(&mut self, from: &ModuleId, to: &ModuleId) {
        self.0.add_edge(from, to).unwrap();
    }

    pub fn remove_module(&mut self, id: &ModuleId) {
        self.0.remove_node(id);
    }

    pub fn get_dependents(&self, id: &ModuleId) -> HashSet<ModuleId> {
        self.0.parents(id).collect()
    }
}

#[derive(Debug, Default)]
pub struct Registry {
    incrementor: Incrementor,
    lookup: BiMap<ModuleId, PathBuf>,
}

impl Registry {
    pub fn upsert<T>(&mut self, path: T) -> ModuleId
    where
        T: AsRef<Path>,
    {
        self.get_id(&path).unwrap_or_else(|| {
            let id = ModuleId(self.incrementor.increment());

            self.lookup.insert(id, path.as_ref().to_path_buf());

            id
        })
    }

    pub fn get_id<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.lookup.get_by_right(path.as_ref()).copied()
    }

    pub fn get_path<T>(&self, id: T) -> Option<PathBuf>
    where
        T: AsRef<ModuleId>,
    {
        self.lookup.get_by_left(id.as_ref()).cloned()
    }
}

#[derive(Debug)]
pub struct State<'a, Log> {
    /// context of the engine that created this state
    pub context: &'a Context<Log>,

    /// scope of the plan that is operating on this state
    pub scope: Scope,

    pub registry: Registry,

    /// represents the dependencies of all modules on each other
    dependencies: Dependencies,

    modules: HashMap<ModuleId, Module>,
}

impl<'a, Log> State<'a, Log> {
    pub const fn log(&self) -> &Log {
        &self.context.logger
    }

    pub const fn graph(&self) -> &Graph {
        &self.dependencies.0
    }

    pub fn new(context: &'a Context<Log>, scope: Scope) -> Self {
        let mut state = Self {
            context,
            scope: scope.clone(),
            dependencies: Dependencies::default(),
            registry: Registry::default(),
            modules: HashMap::default(),
        };

        for path in scope {
            state.registry.upsert(path.clone());
        }

        state
    }

    fn identify_live_path<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.registry
            .get_id(path)
            .and_then(|id| match self.modules.get(&id) {
                Some(Module {
                    status: Status::Evicted,
                    ..
                })
                | None => None,

                Some(_) => Some(id),
            })
    }

    fn purge_module_by_id(&mut self, id: &ModuleId) {
        self.dependencies.remove_module(id);
        self.modules.remove(id);
    }

    fn set_status_by_id(&mut self, id: ModuleId, status: Status) {
        self.modules.entry(id).and_modify(|x| {
            x.status = status;
        });
    }

    fn evict_module_by_id(&mut self, id: ModuleId) -> HashSet<ModuleId> {
        let tainted = self.dependencies.get_dependents(&id);

        self.set_status_by_id(id, Status::Evicted);

        for id in &tainted {
            self.set_status_by_id(*id, Status::Stale);
        }

        tainted
    }

    fn evict_module_by_path<T>(&mut self, path: T) -> HashSet<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.identify_live_path(path)
            .map(|id| self.evict_module_by_id(id))
            .unwrap_or_default()
    }

    fn taint_module_by_path<T>(&mut self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.identify_live_path(path)
            .inspect(|id| self.set_status_by_id(*id, Status::Tainted))
    }

    fn purge_evicted(&mut self) {
        let to_purge = self
            .modules
            .values()
            .filter_map(|x| (x.status == Status::Evicted).then_some(x.id))
            .collect::<Vec<_>>();

        for id in to_purge {
            self.purge_module_by_id(&id);
        }
    }

    fn apply_operations<Ops>(&mut self, operations: Ops) -> HashSet<ModuleId>
    where
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        operations
            .as_ref()
            .iter()
            .flat_map(|(path, op)| match op {
                Operation::Create => HashSet::from([self.registry.upsert(path)]),

                Operation::Update => self
                    .taint_module_by_path(path)
                    .map(|id| HashSet::from([id]))
                    .unwrap_or_default(),

                Operation::Delete => self.evict_module_by_path(path),
            })
            .collect()
    }

    pub fn evolve<Ops>(mut self, operations: Ops) -> Self
    where
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        self.purge_evicted();

        let tainted = self.apply_operations(operations);

        self.scope = tainted
            .into_iter()
            .filter_map(|id| self.registry.get_path(id))
            .collect();

        self
    }

    pub fn upsert_module(
        &mut self,
        id: ModuleId,
        path: PathBuf,
        text: String,
        ast: ast::raw::Program,
    ) {
        self.modules.insert(
            id,
            Module {
                id,
                path,
                text,
                raw: Ast::Program(ast),
                typed: None,
                status: Status::Active,
            },
        );
    }

    pub fn get_module(&self, id: &ModuleId) -> Option<&Module> {
        self.modules.get(id)
    }

    pub fn get_absolute_path<T>(&self, path: T) -> PathBuf
    where
        T: AsRef<Path>,
    {
        self.context.root_dir.join(path.as_ref())
    }

    pub fn add_dependency(&mut self, from: &ModuleId, to: &ModuleId) {
        self.dependencies.add_dependency(from, to);
    }

    pub fn load_and_parse_module<T>(&self, path: T) -> (String, ast::raw::Program)
    where
        T: AsRef<Path>,
    {
        let absolute = self.get_absolute_path(path.as_ref());
        let text = fs::read_to_string(absolute).unwrap();

        let (ast, _) = parse::program::parse(&text).unwrap();

        (text, ast)
    }

    #[cfg(test)]
    pub fn mock(context: &'a Context<Log>) -> Self {
        Self::new(context, vec![])
    }

    #[cfg(test)]
    pub fn create_module<T, U>(
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
        let id = self.registry.upsert(&path);

        self.modules.insert(
            id,
            Module {
                id,
                path: path.as_ref().to_path_buf(),
                text: text.as_ref().to_owned(),
                raw: Ast::Program(ast),
                typed: None,
                status,
            },
        );

        id
    }
}
