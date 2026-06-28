use super::{code::ToCode, Display, ErrorCode, ErrorContext, ErrorDisplay};
use kore::{color::Highlight, format::SeparateEach, pretty::Pretty};
use lang::{CanonicalId, Namespace};
use std::path::PathBuf;

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ExecutionError {
    // internal errors
    UnregisteredModule(Namespace),

    // parsing errors
    InvalidSyntax(PathBuf),

    // linking errors
    ModuleNotFound(Namespace),
    ImportCycle(Vec<Namespace>),

    // analysis errors
    AnalysisError(CanonicalId, analyze::Error),
    // generation errors
}

impl ToCode for ExecutionError {
    fn to_code(&self) -> ErrorCode {
        match self {
            // internal errors
            Self::UnregisteredModule(..) => ErrorCode::UNREGISTERED_MODULE,

            // parsing errors
            Self::InvalidSyntax(..) => ErrorCode::INVALID_SYNTAX,

            // linking errors
            Self::ModuleNotFound(..) => ErrorCode::MODULE_NOT_FOUND,
            Self::ImportCycle(..) => ErrorCode::IMPORT_CYCLE,

            // analysis errors
            Self::AnalysisError(_, err) => err.to_code(),
        }
    }
}

impl<'a> Display<'a> for ExecutionError {
    type Context = &'a ErrorContext;

    fn display(&'a self, context: Self::Context) -> super::ErrorDisplay<'a> {
        let simple = |title, description| ErrorDisplay::simple(self.to_code(), title, description);

        match self {
            // internal errors
            Self::UnregisteredModule(namespace) => simple(
                "Unregistered Module",
                format!(
                    "A referenced module ({}) was not found when linking.

{}",
                    namespace.pretty(),
                    "This should not be possible and represents a fatal internal error.".error()
                ),
            ),

            // parsing errors
            Self::InvalidSyntax(path) => simple(
                "Invalid Syntax",
                format!(
                    "The file {} does not contain valid Knot code.",
                    path.pretty()
                ),
            ),

            // linking errors
            Self::ModuleNotFound(namespace) => simple(
                "Module Not Found",
                format!("Unable to find module {}.", namespace.pretty()),
            ),

            Self::ImportCycle(namespaces) => simple(
                "Import Cycle",
                format!(
                    "An import cycle was found between the following modules:

{}",
                    SeparateEach(
                        format!(" {} ", "->".subtle()),
                        namespaces.iter().map(Pretty::pretty)
                    )
                ),
            ),

            // analysis errors
            Self::AnalysisError(id, err) => err.display((id, context)),
        }
    }
}
