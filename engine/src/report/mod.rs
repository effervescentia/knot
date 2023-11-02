mod code_frame;
mod error;
mod reporter;

pub use code_frame::CodeFrame;
pub use error::Error;
pub use reporter::Reporter;

pub trait Errors {
    type Iter: Iterator<Item = Error>;

    fn errors(self) -> Self::Iter;
}
