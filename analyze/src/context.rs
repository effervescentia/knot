use crate::AmbientScope;
use kore::Serializable;
use lang::{ast, CanonicalId, Canonicalize, Namespace, NamespaceId, NodeId};
use std::{collections::HashMap, fmt::Debug, rc::Rc};

pub type TypeMap = HashMap<CanonicalId, Rc<ast::typed::Meta>>;
pub type AmbientMap = HashMap<AmbientScope, NamespaceId>;
pub type LibraryMap<T> = HashMap<String, T>;

#[derive(Clone, Debug, PartialEq)]
pub struct ModuleMap<Library>
where
    Library: Serializable,
{
    pub keys: HashMap<Namespace<Library>, NamespaceId>,

    #[allow(clippy::type_complexity)]
    pub by_key: HashMap<NamespaceId, (CanonicalId, HashMap<String, CanonicalId>, TypeMap)>,
}

impl<Library> ModuleMap<Library>
where
    Library: Serializable,
{
    pub fn get_type(&self, id: CanonicalId) -> Option<&Rc<ast::typed::Meta>> {
        self.by_key
            .get(&id.0)
            .and_then(|(.., types)| types.get(&id))
    }

    pub fn get_module_type(&self, namespace: &Namespace<Library>) -> Option<&Rc<ast::typed::Meta>> {
        let key = self.keys.get(namespace)?;

        self.by_key
            .get(key)
            .and_then(|(id, _, types)| types.get(id))
    }

    pub fn get_export_type(
        &self,
        namespace: &NamespaceId,
        name: &str,
    ) -> Option<&Rc<ast::typed::Meta>> {
        let (_, exports, types) = self.by_key.get(namespace)?;
        let export_id = exports.get(name)?;

        types.get(export_id)
    }
}

impl<Library> Default for ModuleMap<Library>
where
    Library: Serializable,
{
    fn default() -> Self {
        Self {
            keys: Default::default(),
            by_key: Default::default(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct Context<'a, Library>
where
    Library: Serializable,
{
    pub id: NamespaceId,

    pub namespace: &'a Namespace<Library>,

    pub modules: &'a ModuleMap<Library>,

    pub ambient: &'a AmbientMap,
}

impl<'a, Library> Context<'a, Library>
where
    Library: Serializable + 'static,
{
    #[cfg(feature = "test")]
    pub const fn mock(modules: &'a ModuleMap<Library>, ambient: &'a AmbientMap) -> Self {
        Self {
            modules,
            ambient,
            id: NamespaceId(0),
            namespace: Namespace::MOCK,
        }
    }
}

impl<'a, Library> Canonicalize for Context<'a, Library>
where
    Library: Serializable,
{
    fn canonicalize(&self, id: NodeId) -> CanonicalId {
        CanonicalId(self.id, id)
    }
}
