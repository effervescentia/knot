mod code_frame;
mod reporter;

use crate::Link;
pub use code_frame::CodeFrame;
pub use reporter::Reporter;
use std::{io, iter::once, path::PathBuf};

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Error {
    // internal errors
    UnregisteredModule(Link),

    // environment errors
    InvalidWriteTarget(PathBuf, io::ErrorKind),

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

            // parsing errors
            Self::InvalidSyntax(..) => ErrorCode::INVALID_SYNTAX,

            // linking errors
            Self::ModuleNotFound(..) => ErrorCode::MODULE_NOT_FOUND,
            Self::ImportCycle(..) => ErrorCode::IMPORT_CYCLE,
        }
    }
}

impl IntoIterator for Error {
    type Item = Self;
    type IntoIter = std::iter::Once<Self::Item>;

    fn into_iter(self) -> Self::IntoIter {
        once(self)
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

    // 2xx - parsing errors

    pub const INVALID_SYNTAX: Self = Self(200);

    // 3xx - linking errors

    pub const MODULE_NOT_FOUND: Self = Self(300);
    pub const IMPORT_CYCLE: Self = Self(301);

    // 4xx - analysis errors

    // 5xx - generation errors
}
