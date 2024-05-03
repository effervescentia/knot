mod code_frame;
mod error;
mod reporter;

pub use code_frame::CodeFrame;
pub use error::Error;
pub use reporter::Reporter;
use std::iter::once;

pub trait Errors {
    type Iter: Iterator<Item = Error>;

    fn errors(self) -> Self::Iter;
}

impl Errors for Error {
    type Iter = std::iter::Once<Self>;

    fn errors(self) -> Self::Iter {
        once(self)
    }
}

impl Errors for Vec<Error> {
    type Iter = std::vec::IntoIter<Error>;

    fn errors(self) -> Self::Iter {
        self.into_iter()
    }
}

impl Errors for crate::Result<()> {
    type Iter = std::vec::IntoIter<Error>;

    fn errors(self) -> Self::Iter {
        match self {
            Ok(()) => vec![],
            Err(errs) => errs,
        }
        .errors()
    }
}
