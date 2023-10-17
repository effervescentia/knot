pub mod analyzer;
pub mod ast;
mod common;
pub mod formatter;
pub mod parser;
#[cfg(feature = "test")]
pub mod test;

pub use common::node::Node;
pub use common::position::Position;
pub use common::range::Range;

#[derive(Debug, PartialEq)]
pub struct Program<C>(pub ast::ModuleNode<C>);
