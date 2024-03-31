pub mod bindings;
pub mod strong;
pub mod weak;

pub use bindings::BindingMap;
use lang::{types::Kind, NodeId, ScopeId};

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor {
    pub id: NodeId,

    pub scope: ScopeId,

    pub kind: Kind,

    pub weak: weak::Type,
}

impl NodeDescriptor {
    pub fn into_inherit_from(self, from_id: NodeId) -> Self {
        Self {
            weak: weak::Type::Inherit(from_id),
            ..self
        }
    }
}

impl bindings::ResolveTarget for NodeDescriptor {
    fn id(&self) -> &NodeId {
        &self.id
    }

    fn scope(&self) -> &ScopeId {
        &self.scope
    }
}
