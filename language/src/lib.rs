pub mod ast;
mod common;
pub mod format;
#[cfg(feature = "test")]
pub mod test;

pub use common::node::Node;
pub use common::position::Position;
pub use common::range::Range;
use std::fmt::{Display, Formatter};

pub trait Identity<T> {
    fn id(&self) -> &T;
}

#[derive(Debug, PartialEq)]
pub struct Program<C>(pub ast::ModuleNode<C>);

impl<C> Display for Program<C> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{program}", program = self.0)
    }
}
