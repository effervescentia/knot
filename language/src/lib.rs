pub mod ast;
#[cfg(feature = "format")]
pub mod format;
mod fragment;
mod namespace;
mod node;
mod range;
#[cfg(feature = "test")]
pub mod test;
pub mod types;

pub use fragment::{Fragment, FragmentMap};
pub use namespace::{Namespace, NamespaceKind};
pub use node::Node;
pub use range::{Point, Range};

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct NodeId(pub usize);

#[derive(Clone, Debug, Default, Eq, Hash, PartialEq)]
pub struct ScopeId(pub Vec<usize>);

impl ScopeId {
    fn child(&self, next_id: usize) -> Self {
        Self([self.0.clone(), vec![next_id]].concat())
    }
}
