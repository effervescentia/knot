mod code_frame;

use crate::Link;
pub use code_frame::CodeFrame;

#[derive(Clone, Eq, Debug, PartialEq)]
pub enum Error {
    ImportCycle(Vec<Link>),
}
