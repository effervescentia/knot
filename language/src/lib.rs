pub mod ast;
mod ast2;
#[cfg(feature = "format")]
pub mod format;
mod interface;
mod node;
#[cfg(feature = "test")]
pub mod test;

use ast::{ImportNode, Module, ModuleNode};
pub use interface::{ModuleReference, ModuleScope};
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

    pub const fn node(&self) -> &ast::ModuleNode<R, C> {
        &self.0
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
