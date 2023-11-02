use super::Errors;
use crate::{Link, Result};
use std::{io, iter::once, path::PathBuf};

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Error {
    // internal errors
    UnregisteredModule(Link),

    // environment errors
    InvalidWriteTarget(PathBuf, io::ErrorKind),
    InvalidGlob(Vec<String>),

    // parsing errors
    InvalidSyntax(Link),

    // linking errors
    ModuleNotFound(Link),
    ImportCycle(Vec<Link>),
    // analysis errors

    // generation errors
}

impl Error {
    pub const fn code(value: &Self) -> ErrorCode {
        match value {
            // internal errors
            Self::UnregisteredModule(..) => ErrorCode::UNREGISTERED_MODULE,

            // environment errors
            Self::InvalidWriteTarget(.., io::ErrorKind::NotFound) => {
                ErrorCode::INVALID_WRITE_TARGET_NOT_FOUND
            }
            Self::InvalidWriteTarget(.., io::ErrorKind::PermissionDenied) => {
                ErrorCode::INVALID_WRITE_TARGET_PERMISSION_DENIED
            }
            Self::InvalidWriteTarget(..) => ErrorCode::INVALID_WRITE_TARGET,
            Self::InvalidGlob(..) => ErrorCode::INVALID_GLOB,

            // parsing errors
            Self::InvalidSyntax(..) => ErrorCode::INVALID_SYNTAX,

            // linking errors
            Self::ModuleNotFound(..) => ErrorCode::MODULE_NOT_FOUND,
            Self::ImportCycle(..) => ErrorCode::IMPORT_CYCLE,
        }
    }
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

impl Errors for Result<()> {
    type Iter = std::vec::IntoIter<Error>;

    fn errors(self) -> Self::Iter {
        match self {
            Ok(()) => vec![],
            Err(errs) => errs,
        }
        .errors()
    }
}

pub struct ErrorCode(u16);

impl ErrorCode {
    // 0xx - internal errors
    pub const UNREGISTERED_MODULE: Self = Self(000);

    // 1xx - environment errors
    pub const INVALID_WRITE_TARGET: Self = Self(100);
    pub const INVALID_WRITE_TARGET_NOT_FOUND: Self = Self(101);
    pub const INVALID_WRITE_TARGET_PERMISSION_DENIED: Self = Self(102);
    // [103..109] reserved space for addition writing errors
    pub const INVALID_GLOB: Self = Self(110);

    // 2xx - parsing errors

    pub const INVALID_SYNTAX: Self = Self(200);

    // 3xx - linking errors

    pub const MODULE_NOT_FOUND: Self = Self(300);
    pub const IMPORT_CYCLE: Self = Self(301);

    // 4xx - analysis errors

    // 5xx - generation errors
}
