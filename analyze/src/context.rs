use crate::AmbientScope;
use lang::{ast, CanonicalId, Canonicalize, Namespace, NamespaceId, NodeId};
use std::{collections::HashMap, rc::Rc};

pub type TypeMap = HashMap<CanonicalId, Rc<ast::typed::Meta>>;
pub type AmbientMap = HashMap<AmbientScope, NamespaceId>;

#[derive(Clone, Debug, Default, PartialEq)]
pub struct ModuleMap {
    pub keys: HashMap<Namespace, NamespaceId>,

    #[allow(clippy::type_complexity)]
    pub by_key: HashMap<NamespaceId, (CanonicalId, HashMap<String, CanonicalId>, TypeMap)>,
}

impl ModuleMap {
    pub fn get_type(&self, id: CanonicalId) -> Option<&Rc<ast::typed::Meta>> {
        self.by_key
            .get(&id.0)
            .and_then(|(.., types)| types.get(&id))
    }

    pub fn get_module_type(&self, namespace: &Namespace) -> Option<&Rc<ast::typed::Meta>> {
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

#[derive(Debug, PartialEq)]
pub struct Context<'a> {
    pub id: NamespaceId,

    pub namespace: &'a Namespace,

    pub modules: &'a ModuleMap,

    pub ambient: &'a AmbientMap,
}

impl<'a> Context<'a> {
    #[cfg(feature = "test")]
    pub const fn mock(modules: &'a ModuleMap, ambient: &'a AmbientMap) -> Self {
        Self {
            modules,
            ambient,
            id: NamespaceId(0),
            namespace: Namespace::MOCK,
        }
    }
}

impl<'a> Canonicalize for Context<'a> {
    fn canonicalize(&self, id: NodeId) -> CanonicalId {
        CanonicalId(self.id, id)
    }
}
