use super::{Context, Operation, Scope};
use bimap::BiMap;
use lang::ast;
use std::{
    collections::{HashMap, HashSet},
    path::{Path, PathBuf},
};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct ModuleId(usize);

impl From<usize> for ModuleId {
    fn from(value: usize) -> Self {
        Self(value)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    /** being processed for the first time */
    Pending,

    /** completely processed and available for use */
    Active,

    /** must be re-analyzed before it can be used */
    Stale,

    /** must be re-loaded and processed before it can be used */
    Tainted,

    /** will be purged on the next incremental execution */
    Evicted,
}

#[derive(Debug, PartialEq)]
pub struct Module {
    pub id: ModuleId,
    pub path: PathBuf,
    pub text: String,
    pub ast: ast::meta::Program<()>,
    pub status: Status,
}

#[derive(Debug, Default)]
struct Dependencies {
    depends_on: HashMap<ModuleId, HashSet<ModuleId>>,
    dependency_of: HashMap<ModuleId, HashSet<ModuleId>>,
}

// TODO: replace this with an actual graph solution
impl Dependencies {
    pub fn add_link(&mut self, from: ModuleId, to: ModuleId) {
        self.depends_on
            .entry(from)
            .and_modify(|x| {
                x.insert(to);
            })
            .or_insert_with(|| HashSet::from([to]));

        self.dependency_of
            .entry(to)
            .and_modify(|x| {
                x.insert(from);
            })
            .or_insert_with(|| HashSet::from([from]));
    }

    pub fn remove_module(&mut self, id: ModuleId) -> HashSet<ModuleId> {
        let depends_on = self.depends_on.remove(&id).unwrap_or_default();

        for dep in depends_on {
            self.dependency_of.entry(dep).and_modify(|x| {
                x.remove(&id);
            });
        }

        self.dependency_of.remove(&id).unwrap_or_default()
    }

    pub fn get_dependents(&self, id: ModuleId) -> HashSet<ModuleId> {
        self.dependency_of.get(&id).cloned().unwrap_or_default()
    }
}

#[derive(Debug)]
pub struct State<'a> {
    /** context of the engine that created this state */
    pub context: &'a Context,

    /** scope of the plan that is operating on this state */
    pub scope: Scope,

    next_id: usize,

    /** contains all dependencies based on the most recently parsed ASTs */
    dependencies: Dependencies,

    /** contains all module ID mappings (even for deleted modules) */
    module_to_path: BiMap<ModuleId, PathBuf>,

    modules: HashMap<ModuleId, Module>,
}

impl<'a> State<'a> {
    pub fn new(context: &'a Context, scope: Scope) -> Self {
        let initial = scope.files.clone();

        let mut state = Self {
            context,
            scope,
            next_id: 0,
            dependencies: Dependencies::default(),
            module_to_path: BiMap::new(),
            modules: HashMap::new(),
        };

        for path in initial {
            state.register_id(path.clone());
        }

        state
    }

    fn register_id<T>(&mut self, path: T) -> ModuleId
    where
        T: AsRef<Path>,
    {
        let id = ModuleId(self.next_id);

        self.next_id += 1;
        self.module_to_path.insert(id, path.as_ref().to_path_buf());

        id
    }

    pub fn identify_path<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.module_to_path.get_by_right(path.as_ref()).copied()
    }

    pub fn identify_live_path<T>(&self, path: T) -> Option<ModuleId>
    where
        T: AsRef<Path>,
    {
        self.identify_path(path)
            .and_then(|id| match self.modules.get(&id) {
                Some(Module {
                    status: Status::Evicted,
                    ..
                })
                | None => None,

                Some(_) => Some(id),
            })
    }

    fn purge_module_by_id(&mut self, id: ModuleId) {
        self.dependencies.remove_module(id);
        self.modules.remove(&id);
    }

    fn set_status_by_id(&mut self, id: ModuleId, status: Status) {
        self.modules.entry(id).and_modify(|x| {
            x.status = status;
        });
    }

    fn evict_module_by_id(&mut self, id: ModuleId) -> HashSet<ModuleId> {
        let tainted = self.dependencies.get_dependents(id);

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
            self.purge_module_by_id(id);
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
                Operation::Create => HashSet::from([self.register_id(path)]),

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

        self.scope = Scope {
            files: tainted
                .into_iter()
                .filter_map(|id| self.module_to_path.get_by_left(&id).cloned())
                .collect(),
            follow_imports: self.scope.follow_imports,
        };

        self
    }

    pub fn upsert_module(
        &mut self,
        id: ModuleId,
        path: PathBuf,
        text: String,
        ast: ast::meta::Program<()>,
    ) {
        self.modules.insert(
            id,
            Module {
                id,
                path,
                text,
                ast,
                status: Status::Pending,
            },
        );
    }

    pub fn get_module(&self, id: &ModuleId) -> Option<&Module> {
        self.modules.get(id)
    }
}
