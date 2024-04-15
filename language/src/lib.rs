pub mod ast;
#[cfg(feature = "format")]
pub mod format;
mod fragment;
mod identify;
mod namespace;
mod node;
mod range;
#[cfg(feature = "test")]
pub mod test;
mod type_of;
pub mod types;
pub mod walk;

use std::fmt::Display;

pub use fragment::{Fragment, FragmentMap};
pub use identify::Identify;
pub use namespace::{Namespace, NamespaceKind};
pub use node::Node;
pub use range::{Point, Range};
pub use type_of::TypeOf;

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct NamespaceId(pub usize);

impl Display for NamespaceId {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        self.0.fmt(f)
    }
}

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct NodeId(pub usize);

#[derive(Clone, Debug, Default, Eq, Hash, PartialEq)]
pub struct ScopeId(pub Vec<usize>);

impl ScopeId {
    fn child(&self, next_id: usize) -> Self {
        Self([self.0.clone(), vec![next_id]].concat())
    }
}

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct CanonicalId(pub NamespaceId, pub NodeId);

impl CanonicalId {
    #[cfg(feature = "test")]
    pub const fn mock(id: usize) -> Self {
        Self(NamespaceId(0), NodeId(id))
    }
}

pub trait Canonicalize {
    fn canonicalize(&self, id: NodeId) -> CanonicalId;
}
