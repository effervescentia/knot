use crate::Link;
use kore::{
    color::{Colorize, Highlight},
    format::SeparateEach,
    pretty::Pretty,
};
use std::{fmt::Display, io, path::PathBuf};

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Error {
    // internal errors
    UnregisteredModule(Link),

    // environment errors
    InvalidWriteTarget(PathBuf, io::ErrorKind),
    InvalidGlob(Vec<String>),
    RootDirectoryNotFound(PathBuf),
    SourceDirectoryNotFound(PathBuf),
    SourceDirectoryNotRelative(PathBuf),
    EntrypointNotFound(PathBuf),
    EntrypointNotRelative(PathBuf),
    CleanupFailed(PathBuf),

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
            Self::SourceDirectoryNotRelative(..) => ErrorCode::SOURCE_DIRECTORY_NOT_RELATIVE,
            Self::EntrypointNotFound(..) => ErrorCode::ENTRYPOINT_NOT_FOUND,
            Self::EntrypointNotRelative(..) => ErrorCode::ENTRYPOINT_NOT_RELATIVE,
            Self::CleanupFailed(..) => ErrorCode::CLEANUP_FAILED,

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
            Self::UnregisteredModule(..) => (
                "Unregistered Module",
                format!(
                    "a referenced module was not found when linking\n\n{}",
                    "(this should not be possible and represents a fatal internal error)".error()
                ),
            ),

            // environment errors
            Self::InvalidWriteTarget(path, error) => (
                "Invalid Write Target",
                format!(
                    "attempted write to {} failed with error {error}",
                    path.pretty()
                ),
            ),

            Self::InvalidGlob(glob) => (
                "Invalid Glob",
                format!(
                    "the provided glob failed to resolve with error(s):\n\n{errors}",
                    errors = SeparateEach(
                        "\n",
                        &glob
                            .iter()
                            .map(|x| format!("\u{2022} {x}"))
                            .collect::<Vec<_>>()
                    )
                ),
            ),

            Self::RootDirectoryNotFound(path) => (
                "Root Directory Not Found",
                format!("no folder was found at the path {}", path.pretty()),
            ),

            Self::SourceDirectoryNotFound(path) => (
                "Source Directory Not Found",
                format!("no folder was found at the path {}", path.pretty()),
            ),

            Self::SourceDirectoryNotRelative(path) => (
                "Source Directory Not Relative",
                format!(
                    "the path to the source directory should be relative to the {} but found {}",
                    "root_dir".highlight(),
                    path.pretty().error()
                ),
            ),

            Self::EntrypointNotFound(path) => (
                "Entrypoint Not Found",
                format!("no module was found at the path {}", path.pretty()),
            ),

            Self::EntrypointNotRelative(path) => (
                "Entrypoint Not Relative",
                format!(
                    "the path to the entrypoint should be relative to the {} but found {}",
                    "source_dir".highlight(),
                    path.pretty().error()
                ),
            ),

            Self::CleanupFailed(path) => (
                "Cleanup Failed",
                format!("unable to delete {} or its contents", path.pretty()),
            ),

            // parsing errors
            Self::InvalidSyntax(link) => (
                "Invalid Syntax",
                format!(
                    "the file {} does not appear to contain valid Knot code",
                    link.to_path().pretty()
                ),
            ),

            // linking errors
            Self::ModuleNotFound(link) => (
                "Module Not Found",
                format!("unable to find module {}", link.to_path().pretty()),
            ),

            Self::ImportCycle(links) => (
                "Import Cycle",
                format!(
                    "an import cycle was found between the following modules:\n\n{}",
                    SeparateEach(
                        &format!(" {} ", "->".subtle()),
                        &links.iter().map(|x| x.to_path().pretty()).collect()
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
    pub const SOURCE_DIRECTORY_NOT_RELATIVE: Self = Self(113);
    pub const ENTRYPOINT_NOT_FOUND: Self = Self(114);
    pub const ENTRYPOINT_NOT_RELATIVE: Self = Self(115);
    pub const CLEANUP_FAILED: Self = Self(116);

    // 2xx - parsing errors

    pub const INVALID_SYNTAX: Self = Self(200);

    // 3xx - linking errors

    pub const MODULE_NOT_FOUND: Self = Self(300);
    pub const IMPORT_CYCLE: Self = Self(301);

    // 4xx - analysis errors

    // 5xx - generation errors
}
