pub mod ast;
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

pub use fragment::{Fragment, FragmentMap};
pub use identify::Identify;
pub use namespace::Namespace;
pub use node::Node;
pub use range::{Point, Range};
use std::fmt::Display;
pub use type_of::TypeOf;

/// unique identifier for each internal or external module
#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct ModuleId(pub usize);

impl Display for ModuleId {
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
pub struct CanonicalId(pub ModuleId, pub NodeId);

impl CanonicalId {
    #[cfg(feature = "test")]
    pub const fn mock(id: usize) -> Self {
        Self(ModuleId(0), NodeId(id))
    }
}

pub trait Canonicalize {
    fn canonicalize(&self, id: NodeId) -> CanonicalId;
}
