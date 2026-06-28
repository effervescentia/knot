mod ast;
mod module;
mod registry;

use crate::{Context, Operation, Scope};
pub use ast::Ast;
pub use module::Module;
#[cfg(test)]
pub use module::Status;
pub use registry::Registry;
use std::{
    fs,
    path::{Path, PathBuf},
};

#[derive(Debug)]
pub struct State<'a, Log> {
    /// context of the engine that created this state
    pub context: &'a Context<Log>,

    /// scope of the plan that is operating on this state
    pub scope: Scope,

    pub modules: Registry,
}

impl<'a, Log> State<'a, Log> {
    pub const fn log(&self) -> &Log {
        &self.context.logger
    }

    // pub const fn graph(&self) -> &Graph {
    //     &self.dependencies.0
    // }

    pub fn new(context: &'a Context<Log>, scope: Scope) -> Self {
        let mut state = Self {
            context,
            scope: scope.clone(),
            modules: Registry::default(),
        };

        for path in scope {
            state.modules.register(path);
        }

        state
    }

    // fn identify_live_path<T>(&self, path: T) -> Option<ModuleId>
    // where
    //     T: AsRef<Path>,
    // {
    //     self.modules.get_by_path(path).and_then(|x| match x {
    //         Module {
    //             status: Status::Evicted,
    //             ..
    //         } => None,

    //         _ => Some(x.id),
    //     })
    // }

    // fn purge_module_by_id(&mut self, id: &ModuleId) {
    //     self.dependencies.remove_module(id);
    //     self.modules.remove(id);
    // }

    // fn set_status_by_id(&mut self, id: ModuleId, status: Status) {
    //     self.modules.entry(id).and_modify(|x| {
    //         x.status = status;
    //     });
    // }

    // fn evict_module_by_id(&mut self, id: ModuleId) -> HashSet<ModuleId> {
    //     let tainted = self.dependencies.get_dependents(&id);

    //     self.set_status_by_id(id, Status::Evicted);

    //     for id in &tainted {
    //         self.set_status_by_id(*id, Status::Stale);
    //     }

    //     tainted
    // }

    // fn evict_module_by_path<T>(&mut self, path: T) -> HashSet<ModuleId>
    // where
    //     T: AsRef<Path>,
    // {
    //     self.identify_live_path(path)
    //         .map(|id| self.evict_module_by_id(id))
    //         .unwrap_or_default()
    // }

    // fn taint_module_by_path<T>(&mut self, path: T) -> Option<ModuleId>
    // where
    //     T: AsRef<Path>,
    // {
    //     self.identify_live_path(path)
    //         .inspect(|id| self.set_status_by_id(*id, Status::Tainted))
    // }

    // fn purge_evicted(&mut self) {
    //     let to_purge = self
    //         .modules
    //         .values()
    //         .filter_map(|x| (x.status == Status::Evicted).then_some(x.id))
    //         .collect::<Vec<_>>();

    //     for id in to_purge {
    //         self.purge_module_by_id(&id);
    //     }
    // }

    // fn apply_operations<Ops>(&mut self, operations: Ops) -> HashSet<ModuleId>
    // where
    //     Ops: AsRef<[(PathBuf, Operation)]>,
    // {
    //     operations
    //         .as_ref()
    //         .iter()
    //         .flat_map(|(path, op)| match op {
    //             Operation::Create => HashSet::from([self.registry.upsert(path)]),

    //             Operation::Update => self
    //                 .taint_module_by_path(path)
    //                 .map(|id| HashSet::from([id]))
    //                 .unwrap_or_default(),

    //             Operation::Delete => self.evict_module_by_path(path),
    //         })
    //         .collect()
    // }

    pub fn evolve<Ops>(mut self, operations: Ops) -> Self
    where
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        // self.purge_evicted();

        // let tainted = self.apply_operations(operations);

        // self.scope = tainted
        //     .into_iter()
        //     .filter_map(|id| self.registry.get_path(id))
        //     .collect();

        self
    }

    pub fn get_absolute_path<T>(&self, path: T) -> PathBuf
    where
        T: AsRef<Path>,
    {
        self.context.root_dir.join(path.as_ref())
    }

    pub fn load_and_parse_module<T>(&self, path: T) -> (String, lang::ast::raw::Program)
    where
        T: AsRef<Path>,
    {
        let absolute = self.get_absolute_path(path.as_ref());
        let text = fs::read_to_string(&absolute)
            .unwrap_or_else(|err| panic!("failed to load module with path {absolute:?}: {err}"));

        let (ast, _) = parse::program::parse(&text).unwrap();

        (text, ast)
    }

    #[cfg(test)]
    pub fn mock(context: &'a Context<Log>) -> Self {
        Self::new(context, vec![])
    }
}
