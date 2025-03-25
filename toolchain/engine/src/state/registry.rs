use super::Module;
use bimap::BiMap;
use kore::{
    graph::{Cycle, Graph},
    Incrementor,
};
use lang::ModuleId;
use std::{
    collections::{HashMap, HashSet},
    path::{Path, PathBuf},
};

#[derive(Debug, Default)]
pub struct Registry {
    incrementor: Incrementor,

    paths: BiMap<ModuleId, PathBuf>,

    dependencies: Graph<ModuleId>,

    data: HashMap<ModuleId, Module>,
}

impl Registry {
    pub fn register<T>(&mut self, path: T) -> ModuleId
    where
        T: AsRef<Path>,
    {
        self.get_id_by_path(&path).unwrap_or_else(|| {
            let id = ModuleId(self.incrementor.increment());

            self.paths.insert(id, path.as_ref().to_path_buf());
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
        self.paths.get_by_right(path.as_ref()).copied()
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
        ast: lang::ast::raw::Program,
        status: super::Status,
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
                namespace: lang::Namespace::from_internal_path(path),
                text: text.as_ref().to_owned(),
                raw: super::Ast::Program(ast),
                typed: None,
                status,
            },
        );

        id
    }
}
