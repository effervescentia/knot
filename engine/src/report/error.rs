use crate::Link;
use kore::{
    color::{Colorize, Highlight},
    format::SeparateEach,
    pretty::Pretty,
    str,
};
use lang::CanonicalId;
use std::{fmt::Display, io, path::PathBuf};

fn write_error(
    f: &mut std::fmt::Formatter,
    code: u16,
    title: &str,
    description: &str,
) -> std::fmt::Result {
    write!(
        f,
        "{title} {code}\n\n{description}",
        title = title.error().bold(),
        code = format!("(E#{})", code).subtle(),
    )
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ConfigurationError {
    InvalidGlob(Vec<String>),
    RootDirectoryNotFound(PathBuf),
    SourceDirectoryNotFound(PathBuf),
    SourceDirectoryNotRelative(PathBuf),
    EntrypointNotFound(PathBuf),
    EntrypointNotRelative(PathBuf),
}

impl ConfigurationError {
    pub const fn code(&self) -> ErrorCode {
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

impl Display for ConfigurationError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let code = self.code();

        let (title, description) = match self {
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
        };

        write_error(f, code.0, title, &description)
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ExecutionError {
    // internal errors
    UnregisteredModule(Link),

    // environment errors
    InvalidWriteTarget(PathBuf, io::ErrorKind),
    CleanupFailed(PathBuf),

    // parsing errors
    InvalidSyntax(Link),

    // linking errors
    ModuleNotFound(Link),
    ImportCycle(Vec<Link>),

    // analysis errors
    AnalysisError(CanonicalId, analyze::Error),
    // generation errors
}

impl ExecutionError {
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
            Self::CleanupFailed(..) => ErrorCode::CLEANUP_FAILED,

            // parsing errors
            Self::InvalidSyntax(..) => ErrorCode::INVALID_SYNTAX,

            // linking errors
            Self::ModuleNotFound(..) => ErrorCode::MODULE_NOT_FOUND,
            Self::ImportCycle(..) => ErrorCode::IMPORT_CYCLE,

            // analysis errors
            Self::AnalysisError(_, err) => match err {
                analyze::Error::NotInferrable(_) => ErrorCode::NOT_INFERRABLE,
                analyze::Error::NotFound(_) => ErrorCode::NOT_FOUND,
                analyze::Error::VariantNotFound(..) => ErrorCode::VARIANT_NOT_FOUND,
                analyze::Error::DeclarationNotFound(..) => ErrorCode::DECLARATION_NOT_FOUND,
                analyze::Error::NotIndexable(..) => ErrorCode::NOT_INDEXABLE,
                analyze::Error::PropertyNotFound(..) => ErrorCode::PROPERTY_NOT_FOUND,
                analyze::Error::DuplicateProperty(_) => ErrorCode::DUPLICATE_PROPERTY,
                analyze::Error::NotSpreadable(_) => ErrorCode::NOT_SPREADABLE,
                analyze::Error::UntypedParameter => ErrorCode::UNTYPED_PARAMETER,
                analyze::Error::DefaultValueRejected(_) => ErrorCode::DEFAULT_VALUE_REJECTED,
                analyze::Error::NotCallable(_) => ErrorCode::NOT_CALLABLE,
                analyze::Error::UnexpectedArgument(_) => ErrorCode::UNEXPECTED_ARGUMENT,
                analyze::Error::MissingArgument(_) => ErrorCode::MISSING_ARGUMENT,
                analyze::Error::ArgumentRejected(..) => ErrorCode::ARGUMENT_REJECTED,
                analyze::Error::NotRenderable(_) => ErrorCode::NOT_RENDERABLE,
                analyze::Error::InvalidComponent(_) => ErrorCode::INVALID_COMPONENT,
                analyze::Error::ComponentTypo(..) => ErrorCode::COMPONENT_TYPO,
                analyze::Error::InvalidAttributes(_) => ErrorCode::INVALID_ATTRIBUTES,
                analyze::Error::UnexpectedAttribute(_) => ErrorCode::UNEXPECTED_ATTRIBUTE,
                analyze::Error::MissingAttribute(_) => ErrorCode::MISSING_ATTRIBUTE,
                analyze::Error::AttributeRejected(..) => ErrorCode::ATTRIBUTE_REJECTED,
                analyze::Error::BinaryOperationNotSupported(..) => {
                    ErrorCode::BINARY_OPERATION_NOT_SUPPORTED
                }
                analyze::Error::UnaryOperationNotSupported(..) => {
                    ErrorCode::UNARY_OPERATION_NOT_SUPPORTED
                }
                analyze::Error::UnexpectedKind(..) => ErrorCode::UNEXPECTED_KIND,
            },
        }
    }
}

impl Display for ExecutionError {
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

            // analysis errors
            Self::AnalysisError(_, err) => match err {
                analyze::Error::NotInferrable(_) => (
                    "Not Inferrable",
                    str!("the type of this expression could not be inferred from other types"),
                ),

                analyze::Error::NotFound(name) => (
                    "Not Found",
                    format!(
                        "unable to resolve identifier {} in the local scope or any inherited scope",
                        name.bold()
                    ),
                ),

                analyze::Error::VariantNotFound(_, _) => ("Variant Not Found", format!("")),

                analyze::Error::DeclarationNotFound(_, _) => ("Declaration Not Found", format!("")),

                analyze::Error::NotIndexable(_, _) => ("Not Indexable", format!("")),

                analyze::Error::PropertyNotFound(_, _) => ("Property Not Found", format!("")),

                analyze::Error::DuplicateProperty(_) => ("Duplicate Property", format!("")),

                analyze::Error::NotSpreadable(_) => ("Not Spreadable", format!("")),

                analyze::Error::UntypedParameter => ("Untyped Parameter", format!("")),

                analyze::Error::DefaultValueRejected(_) => ("Default Value Rejected", format!("")),

                analyze::Error::NotCallable(_) => ("Not Callable", format!("")),

                analyze::Error::UnexpectedArgument(_) => ("Unexpected Argument", format!("")),

                analyze::Error::MissingArgument(_) => ("Missing Argument", format!("")),

                analyze::Error::ArgumentRejected(_, _) => ("Argument Rejected", format!("")),

                analyze::Error::NotRenderable(_) => ("Not Renderable", format!("")),

                analyze::Error::InvalidComponent(_) => ("Invalid Component", format!("")),

                analyze::Error::ComponentTypo(_, _) => ("Component Typo", format!("")),

                analyze::Error::InvalidAttributes(_) => ("Invalid Attributes", format!("")),

                analyze::Error::UnexpectedAttribute(_) => ("Unexpected Attribute", format!("")),

                analyze::Error::MissingAttribute(_) => ("Missing Attribute", format!("")),

                analyze::Error::AttributeRejected(_, _) => ("Attribute Rejected", format!("")),

                analyze::Error::BinaryOperationNotSupported(_, _, _) => {
                    ("Binary Operation Not Supported", format!(""))
                }

                analyze::Error::UnaryOperationNotSupported(_, _) => {
                    ("Unary Operation Not Supported", format!(""))
                }

                analyze::Error::UnexpectedKind(_, _) => ("Unexpected Kind", format!("")),
            },
        };

        write_error(f, code.0, title, &description)
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

    pub const NOT_INFERRABLE: Self = Self(400);
    pub const NOT_FOUND: Self = Self(401);
    pub const VARIANT_NOT_FOUND: Self = Self(402);
    pub const DECLARATION_NOT_FOUND: Self = Self(403);
    pub const NOT_INDEXABLE: Self = Self(404);
    pub const PROPERTY_NOT_FOUND: Self = Self(405);
    pub const DUPLICATE_PROPERTY: Self = Self(406);
    pub const NOT_SPREADABLE: Self = Self(407);
    pub const UNTYPED_PARAMETER: Self = Self(408);
    pub const DEFAULT_VALUE_REJECTED: Self = Self(409);
    pub const NOT_CALLABLE: Self = Self(410);
    pub const UNEXPECTED_ARGUMENT: Self = Self(411);
    pub const MISSING_ARGUMENT: Self = Self(412);
    pub const ARGUMENT_REJECTED: Self = Self(413);
    pub const NOT_RENDERABLE: Self = Self(414);
    pub const INVALID_COMPONENT: Self = Self(415);
    pub const COMPONENT_TYPO: Self = Self(416);
    pub const INVALID_ATTRIBUTES: Self = Self(417);
    pub const UNEXPECTED_ATTRIBUTE: Self = Self(418);
    pub const MISSING_ATTRIBUTE: Self = Self(419);
    pub const ATTRIBUTE_REJECTED: Self = Self(420);
    pub const BINARY_OPERATION_NOT_SUPPORTED: Self = Self(421);
    pub const UNARY_OPERATION_NOT_SUPPORTED: Self = Self(422);
    pub const UNEXPECTED_KIND: Self = Self(423);

    // 5xx - generation errors
}
