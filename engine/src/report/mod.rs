mod code_frame;
mod error;
mod into_errors;
mod reporter;

use crate::Link;
pub use code_frame::CodeFrame;
use error::Display;
pub use error::{ConfigurationError, EnvironmentError, ExecutionError};
pub use into_errors::IntoErrors;
use kore::color::{ColoredString, Colorize, Highlight};
use lang::{CanonicalId, NamespaceId, Range};
pub use reporter::Reporter;
use std::collections::HashMap;

pub trait Enrich {
    fn enrich(&self, failure: Failure) -> Report {
        failure.no_context()
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Report {
    Configuration(ConfigurationError),
    Environment(EnvironmentError),
    Execution {
        root_dir: String,
        modules: HashMap<NamespaceId, (Link, String)>,
        nodes: HashMap<CanonicalId, Range>,
        errors: Vec<ExecutionError>,
    },
}

impl Report {
    #[cfg(feature = "test")]
    pub const fn exec_errors(&self) -> Option<&Vec<ExecutionError>> {
        match self {
            Self::Execution { errors, .. } => Some(errors),
            Self::Configuration(_) => None,
            Self::Environment(_) => None,
        }
    }
}

impl std::fmt::Display for Report {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        fn format_bumper(error_count: usize) -> ColoredString {
            format!("finished with {} error(s)", error_count.to_string().bold()).error()
        }

        fn write_single<T>(f: &mut std::fmt::Formatter, error: T) -> std::fmt::Result
        where
            T: std::fmt::Display,
        {
            let bumper = format_bumper(1);

            writeln!(f, "{}\n", bumper)?;
            writeln!(f, "{index} {error}\n", index = format!("{})", 1).error())?;
            writeln!(f, "{}\n", bumper)
        }

        match self {
            Self::Configuration(error) => write_single(f, error.display(())),

            Self::Environment(error) => write_single(f, error.display(())),

            Self::Execution {
                errors,
                modules,
                nodes,
                ..
            } => {
                let errors = errors
                    .iter()
                    .filter(|error| {
                        !matches!(
                            error,
                            ExecutionError::AnalysisError(_, analyze::Error::NotInferrable(_))
                        )
                    })
                    .collect::<Vec<_>>();

                let bumper = format_bumper(errors.len());

                writeln!(f, "{}\n", bumper)?;

                for (index, error) in errors.iter().enumerate() {
                    writeln!(
                        f,
                        "{index} {error}\n",
                        index = format!("{})", index + 1).error(),
                        error = error.display((modules, nodes))
                    )?;
                }

                writeln!(f, "{}\n", bumper)
            }
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Failure {
    Execution(Vec<ExecutionError>),
}

impl Failure {
    pub fn no_context(self) -> Report {
        match self {
            Self::Execution(errors) => Report::Execution {
                root_dir: Default::default(),
                modules: Default::default(),
                nodes: Default::default(),
                errors,
            },
        }
    }
}
