pub mod ast;
#[cfg(feature = "format")]
pub mod format;
mod interface;
mod node;
mod range;
#[cfg(test)]
mod test;
pub mod types;

pub use interface::{ModuleReference, ModuleScope};
pub use node::Node;
pub use range::{Point, Range};

// impl<R, C> Program<R, C> {
//     pub const fn imports(&self) -> &Vec<ImportNode<R, C>> {
//         let Self(ModuleNode(Module { imports, .. }, ..)) = self;

//         imports
//     }
// }
