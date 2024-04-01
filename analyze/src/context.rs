use lang::{ast, CanonicalId, Canonicalize, Namespace, NamespaceId, NodeId};
use std::{collections::HashMap, rc::Rc};

#[derive(Debug, Default, PartialEq)]
pub struct ModuleMap {
    pub keys: HashMap<Namespace, NamespaceId>,

    #[allow(clippy::type_complexity)]
    pub by_key: HashMap<
        NamespaceId,
        (
            CanonicalId,
            // TODO: drop this since the type can be looked up in the table below with the ID above
            ast::typed::Type,
            HashMap<CanonicalId, Rc<(CanonicalId, ast::typed::Type)>>,
        ),
    >,
}

impl ModuleMap {
    pub fn resolve(&self, id: CanonicalId) -> Option<&Rc<(CanonicalId, ast::typed::Type)>> {
        self.by_key
            .get(&id.0)
            .and_then(|(_, _, types)| types.get(&id))
    }

    pub fn resolve_module(
        &self,
        namespace: &Namespace,
    ) -> Option<&Rc<(CanonicalId, ast::typed::Type)>> {
        let key = self.keys.get(namespace)?;

        self.by_key
            .get(key)
            .and_then(|(id, _, types)| types.get(id))
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
