use super::{code::ToCode, Display, ErrorCode, ErrorContext, ErrorDisplay};
use crate::Link;
use kore::{color::Highlight, format::SeparateEach, pretty::Pretty};
use lang::CanonicalId;

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ExecutionError {
    // internal errors
    UnregisteredModule(Link),

    // parsing errors
    InvalidSyntax(Link),

    // linking errors
    ModuleNotFound(Link),
    ImportCycle(Vec<Link>),

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
            Self::UnregisteredModule(link) => simple(
                "Unregistered Module",
                format!(
                    "A referenced module ({}) was not found when linking.

{}",
                    link.to_path().pretty(),
                    "This should not be possible and represents a fatal internal error.".error()
                ),
            ),

            // parsing errors
            Self::InvalidSyntax(link) => simple(
                "Invalid Syntax",
                format!(
                    "The file {} does not contain valid Knot code.",
                    link.to_path().pretty()
                ),
            ),

            // linking errors
            Self::ModuleNotFound(link) => simple(
                "Module Not Found",
                format!("Unable to find module {}.", link.to_path().pretty()),
            ),

            Self::ImportCycle(links) => simple(
                "Import Cycle",
                format!(
                    "An import cycle was found between the following modules:

{}",
                    SeparateEach(
                        format!(" {} ", "->".subtle()),
                        links.iter().map(|x| x.to_path().pretty())
                    )
                ),
            ),

            // analysis errors
            Self::AnalysisError(id, err) => err.display((id, context)),
        }
    }
}
