use super::Module;
use crate::{Library, Link};
use bimap::BiMap;
use lang::NamespaceId;
use std::collections::HashMap;

pub type ModuleIterator<'a, T> =
    Box<dyn std::iter::Iterator<Item = (&'a Link, &'a super::Module<T>)> + 'a>;

#[derive(Clone, Default)]
pub struct Base<T> {
    modules: HashMap<Link, super::Module<T>>,
    lookup: BiMap<Link, NamespaceId>,
    ambient: analyze::AmbientMap,
}

impl<T> Base<T> {
    pub const fn ambient(&self) -> &analyze::AmbientMap {
        &self.ambient
    }

    pub fn has_by_link(&self, link: &Link) -> bool {
        self.modules.contains_key(link)
    }

    pub fn get_id_by_link(&self, link: &Link) -> Option<&NamespaceId> {
        self.lookup.get_by_left(link)
    }

    pub fn get_link_by_id(&self, id: &NamespaceId) -> Option<&Link> {
        self.lookup.get_by_right(id)
    }

    pub fn get_module_by_link(&self, link: &Link) -> Option<&Module<T>> {
        self.modules.get(link)
    }

    pub fn get_link_and_module_by_id(&self, id: &NamespaceId) -> Option<(&Link, &Module<T>)> {
        let link = self.get_link_by_id(id)?;

        Some((link, self.modules.get(link)?))
    }

    pub fn modules(&self) -> ModuleIterator<T> {
        Box::new(self.modules.iter())
    }

    pub fn internal_modules(&self) -> ModuleIterator<T> {
        Box::new(
            self.modules
                .iter()
                .filter_map(|(link, module)| link.is_internal().then_some((link, module))),
        )
    }

    pub fn register_library(&mut self, library: Library, link: Link, module: Module<T>) {
        if let Some(scope) = library.to_ambient_scope() {
            self.ambient.insert(scope, module.id);
        }

        self.modules.insert(link, module);
    }

    pub fn register_module(&mut self, link: Link, module: Module<T>) {
        self.lookup.insert(link.clone(), module.id);
        self.modules.insert(link, module);
    }

    pub fn with_modules<R>(self, modules: HashMap<Link, super::Module<R>>) -> Base<R> {
        Base {
            modules,
            lookup: self.lookup,
            ambient: self.ambient,
        }
    }
}
