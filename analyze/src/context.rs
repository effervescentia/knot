use lang::{ast, CanonicalId, Canonicalize, Namespace, NamespaceId, NodeId};
use std::{collections::HashMap, rc::Rc};

pub type TypeMap = HashMap<CanonicalId, Rc<ast::typed::Meta>>;

#[derive(Debug, Default, PartialEq)]
pub struct ModuleMap {
    pub keys: HashMap<Namespace, NamespaceId>,

    #[allow(clippy::type_complexity)]
    pub by_key: HashMap<NamespaceId, (CanonicalId, TypeMap)>,
}

impl ModuleMap {
    pub fn resolve(&self, id: CanonicalId) -> Option<&Rc<ast::typed::Meta>> {
        self.by_key.get(&id.0).and_then(|(_, types)| types.get(&id))
    }

    pub fn resolve_module(&self, namespace: &Namespace) -> Option<&Rc<ast::typed::Meta>> {
        let key = self.keys.get(namespace)?;

        self.by_key.get(key).and_then(|(id, types)| types.get(id))
    }
}

#[derive(Debug, PartialEq)]
pub struct Context<'a> {
    pub namespace_id: NamespaceId,

    pub namespace: &'a Namespace,

    pub modules: &'a ModuleMap,
}

impl<'a> Context<'a> {
    #[cfg(test)]
    pub const fn mock(modules: &'a ModuleMap) -> Self {
        Self {
            modules,
            namespace_id: NamespaceId(0),
            namespace: Namespace::MOCK,
        }
    }
}

impl<'a> Canonicalize for Context<'a> {
    fn canonicalize(&self, id: NodeId) -> CanonicalId {
        CanonicalId(self.namespace_id, id)
    }
}
