pub mod ast;
#[cfg(feature = "format")]
pub mod format;
mod node;
#[cfg(feature = "test")]
pub mod test;

use ast::{ImportNode, Module, ModuleNode};
pub use node::Node;

pub trait Identity<T> {
    fn id(&self) -> &T;
}

#[derive(Debug, PartialEq)]
pub struct Program<R, C>(pub ast::ModuleNode<R, C>);

impl<R, C> Program<R, C> {
    pub const fn imports(&self) -> &Vec<ImportNode<R, C>> {
        let Self(ModuleNode(Module { imports, .. }, ..)) = self;

        imports
    }
}

#[cfg(feature = "format")]
impl<R, C> std::fmt::Display for Program<R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{program}", program = self.0)
    }
}
