use crate::Link;
use kore::{
    color::{Colorize, Highlight},
    format::SeparateEach,
};
use std::{fmt::Display, io, path::PathBuf};

trait Pretty {
    fn pretty(&self) -> String;
}

impl Pretty for PathBuf {
    fn pretty(&self) -> String {
        // Self::from(".").join(self).to_string_lossy().to_string()
        self.to_string_lossy().to_string()
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Error {
    // internal errors
    UnregisteredModule(Link),

    // environment errors
    InvalidWriteTarget(PathBuf, io::ErrorKind),
    InvalidGlob(Vec<String>),
    RootDirectoryNotFound(PathBuf),
    SourceDirectoryNotFound(PathBuf),
    EntrypointNotFound(PathBuf),

    // parsing errors
    InvalidSyntax(Link),

    // linking errors
    ModuleNotFound(Link),
    ImportCycle(Vec<Link>),
    // analysis errors

    // generation errors
}

impl Error {
    pub const fn code(&self) -> ErrorCode {
        match self {
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
            Self::RootDirectoryNotFound(..) => ErrorCode::ROOT_DIRECTORY_NOT_FOUND,
            Self::SourceDirectoryNotFound(..) => ErrorCode::SOURCE_DIRECTORY_NOT_FOUND,
            Self::EntrypointNotFound(..) => ErrorCode::ENTRYPOINT_NOT_FOUND,

            // parsing errors
            Self::InvalidSyntax(..) => ErrorCode::INVALID_SYNTAX,

            // linking errors
            Self::ModuleNotFound(..) => ErrorCode::MODULE_NOT_FOUND,
            Self::ImportCycle(..) => ErrorCode::IMPORT_CYCLE,
        }
    }
}

impl Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let code = self.code();

        let (title, description) = match self {
            // internal errors
            Self::UnregisteredModule(..) => ("Unregistered Module", format!("")),

            // environment errors
            Self::InvalidWriteTarget(path, err) => (
                "Invalid Write Target",
                format!(
                    "attempted write to {} failed with error {err}",
                    path.pretty()
                ),
            ),

            Self::InvalidGlob(..) => ("Invalid Glob", format!("")),

            Self::RootDirectoryNotFound(path) => (
                "Root Directory Not Found",
                format!(
                    "no folder was found at the path {}",
                    path.pretty().highlight()
                ),
            ),

            Self::SourceDirectoryNotFound(path) => (
                "Source Directory Not Found",
                format!(
                    "no folder was found at the path {}",
                    path.pretty().highlight()
                ),
            ),

            Self::EntrypointNotFound(path) => (
                "Entrypoint Not Found",
                format!(
                    "no module was found at the path {}",
                    path.pretty().highlight()
                ),
            ),

            // parsing errors
            Self::InvalidSyntax(link) => (
                "Invalid Syntax",
                format!(
                    "the file {} does not appear to contain valid Knot code",
                    link.to_path().pretty().highlight()
                ),
            ),

            // linking errors
            Self::ModuleNotFound(link) => (
                "Module Not Found",
                format!(
                    "unable to find module {}",
                    link.to_path().pretty().highlight()
                ),
            ),

            Self::ImportCycle(links) => (
                "Import Cycle",
                format!(
                    "an import cycle was found between the following modules:\n\n{}",
                    SeparateEach(
                        &format!(" {} ", "->".subtle()),
                        &links
                            .iter()
                            .map(|x| x.to_path().pretty().highlight())
                            .collect()
                    )
                ),
            ),
        };

        write!(
            f,
            "{title} {code}\n\n{description}",
            title = title.error().bold(),
            code = format!("(E#{})", code.0).subtle(),
        )
    }
}

#[derive(Debug)]
pub struct ErrorCode(u16);

impl ErrorCode {
    // 0xx - internal errors

    pub const UNREGISTERED_MODULE: Self = Self(000);

    // 1xx - environment errors

    pub const INVALID_WRITE_TARGET: Self = Self(100);
    pub const INVALID_WRITE_TARGET_NOT_FOUND: Self = Self(101);
    pub const INVALID_WRITE_TARGET_PERMISSION_DENIED: Self = Self(102);
    // [103..109] reserved space for additional writing errors
    pub const INVALID_GLOB: Self = Self(110);
    pub const ROOT_DIRECTORY_NOT_FOUND: Self = Self(111);
    pub const SOURCE_DIRECTORY_NOT_FOUND: Self = Self(112);
    pub const ENTRYPOINT_NOT_FOUND: Self = Self(113);

    // 2xx - parsing errors

    pub const INVALID_SYNTAX: Self = Self(200);

    // 3xx - linking errors

    pub const MODULE_NOT_FOUND: Self = Self(300);
    pub const IMPORT_CYCLE: Self = Self(301);

    // 4xx - analysis errors

    // 5xx - generation errors
}
