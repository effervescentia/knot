pub mod bindings;
pub mod strong;
pub mod weak;

pub use bindings::BindingMap;
use kore::invariant;
use lang::{types::Kind, CanonicalId, NodeId, ScopeId};

/// interchange format between weak and strong inference phases
#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor {
    pub id: CanonicalId,

    pub scope: ScopeId,

    pub kind: Kind,

    pub weak: weak::Type,
}

impl NodeDescriptor {
    pub fn into_inherit_from(self, from_id: CanonicalId) -> Self {
        if self.id.0 == from_id.0 {
            invariant!("redirecting inheritance outside of the target module")
        }

        Self {
            weak: weak::Type::Inherit(from_id.1),
            ..self
        }
    }
}

impl bindings::ResolveTarget for NodeDescriptor {
    fn id(&self) -> &NodeId {
        &self.id.1
    }

    fn scope(&self) -> &ScopeId {
        &self.scope
    }
}
