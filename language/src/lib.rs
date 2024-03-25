pub mod ast;
#[cfg(feature = "format")]
pub mod format;
mod fragment;
mod interface;
mod node;
mod range;
#[cfg(feature = "test")]
pub mod test;
pub mod types;

pub use fragment::{Fragment, FragmentMap};
pub use interface::{ModuleReference, ModuleScope};
pub use node::Node;
pub use range::{Point, Range};

// impl<R, C> Program<R, C> {
//     pub const fn imports(&self) -> &Vec<ImportNode<R, C>> {
//         let Self(ModuleNode(Module { imports, .. }, ..)) = self;

//         imports
//     }
// }

#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct NodeId(pub usize);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ScopeId(pub Vec<usize>);

impl ScopeId {
    fn child(&self, next_id: usize) -> Self {
        Self([self.0.clone(), vec![next_id]].concat())
    }
}

impl Default for ScopeId {
    fn default() -> Self {
        Self(vec![0])
    }
}
