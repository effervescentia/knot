use super::{code::ToCode, Display, ErrorCode, ErrorDisplay};
use kore::{color::Highlight, format::SeparateEach, pretty::Pretty};
use std::path::PathBuf;

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ConfigurationError {
    InvalidGlob(Vec<String>),
    RootDirectoryNotFound(PathBuf),
    SourceDirectoryNotFound(PathBuf),
    SourceDirectoryNotRelative(PathBuf),
    EntrypointNotFound(PathBuf),
    EntrypointNotRelative(PathBuf),
}

impl ToCode for ConfigurationError {
    fn to_code(&self) -> ErrorCode {
        match self {
            Self::InvalidGlob(..) => ErrorCode::INVALID_GLOB,
            Self::RootDirectoryNotFound(..) => ErrorCode::ROOT_DIRECTORY_NOT_FOUND,
            Self::SourceDirectoryNotFound(..) => ErrorCode::SOURCE_DIRECTORY_NOT_FOUND,
            Self::SourceDirectoryNotRelative(..) => ErrorCode::SOURCE_DIRECTORY_NOT_RELATIVE,
            Self::EntrypointNotFound(..) => ErrorCode::ENTRYPOINT_NOT_FOUND,
            Self::EntrypointNotRelative(..) => ErrorCode::ENTRYPOINT_NOT_RELATIVE,
        }
    }
}

impl<'a> Display<'a> for ConfigurationError {
    type Context = ();

    fn display(&'a self, (): Self::Context) -> ErrorDisplay<'a> {
        let bind = |title, description| ErrorDisplay {
            code: self.to_code(),
            title,
            description,
            code_frame: None,
        };

        match self {
            Self::InvalidGlob(glob) => bind(
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

            Self::RootDirectoryNotFound(path) => bind(
                "Root Directory Not Found",
                format!("no folder was found at the path {}", path.pretty()),
            ),

            Self::SourceDirectoryNotFound(path) => bind(
                "Source Directory Not Found",
                format!("no folder was found at the path {}", path.pretty()),
            ),

            Self::SourceDirectoryNotRelative(path) => bind(
                "Source Directory Not Relative",
                format!(
                    "the path to the source directory should be relative to the {} but found {}",
                    "root_dir".highlight(),
                    path.pretty().error()
                ),
            ),

            Self::EntrypointNotFound(path) => bind(
                "Entrypoint Not Found",
                format!("no module was found at the path {}", path.pretty()),
            ),

            Self::EntrypointNotRelative(path) => bind(
                "Entrypoint Not Relative",
                format!(
                    "the path to the entrypoint should be relative to the {} but found {}",
                    "source_dir".highlight(),
                    path.pretty().error()
                ),
            ),
        }
    }
}
