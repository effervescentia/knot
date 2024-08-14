use super::{code::ToCode, Display, ErrorCode, ErrorDisplay};
use crate::Link;
use kore::{color::Highlight, format::SeparateEach, pretty::Pretty};
use lang::CanonicalId;
use std::collections::HashMap;

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
    type Context = (
        &'a str,
        &'a HashMap<lang::NamespaceId, (Link, String)>,
        &'a HashMap<lang::CanonicalId, lang::Range>,
    );

    fn display(&'a self, (root_dir, modules, nodes): Self::Context) -> super::ErrorDisplay<'a> {
        let simple = |title, description| ErrorDisplay::simple(self.to_code(), title, description);

        match self {
            // internal errors
            Self::UnregisteredModule(link) => simple(
                "Unregistered Module",
                format!(
                    "a referenced module ({}) was not found when linking\n\n{}",
                    link.to_path().pretty(),
                    "(this should not be possible and represents a fatal internal error)".error()
                ),
            ),

            // parsing errors
            Self::InvalidSyntax(link) => simple(
                "Invalid Syntax",
                format!(
                    "the file {} does not contain valid Knot code",
                    link.to_path().pretty()
                ),
            ),

            // linking errors
            Self::ModuleNotFound(link) => simple(
                "Module Not Found",
                format!("unable to find module {}", link.to_path().pretty()),
            ),

            Self::ImportCycle(links) => simple(
                "Import Cycle",
                format!(
                    "an import cycle was found between the following modules:\n\n{}",
                    SeparateEach(
                        &format!(" {} ", "->".subtle()),
                        &links.iter().map(|x| x.to_path().pretty()).collect()
                    )
                ),
            ),

            // analysis errors
            Self::AnalysisError(id, err) => err.display((id, root_dir, modules, nodes)),
        }
    }
}
