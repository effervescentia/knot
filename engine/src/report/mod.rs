mod code_frame;
mod error;
mod example;
mod into_errors;
mod reporter;

pub use code_frame::CodeFrame;
use error::Display;
pub use error::{ConfigurationError, EnvironmentError, ErrorContext, ExecutionError};
pub use into_errors::IntoErrors;
use kore::color::{ColoredString, Colorize, Highlight};
pub use reporter::Reporter;

pub trait Enrich {
    fn enrich(&self, root_dir: String, failure: Failure) -> Report {
        failure.no_context(root_dir)
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Report {
    Configuration(ConfigurationError),
    Environment(EnvironmentError),
    Execution {
        errors: Vec<ExecutionError>,
        context: ErrorContext,
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
            format!(
                "finished with {} {}",
                error_count.to_string().bold(),
                if error_count > 1 { "errors" } else { "error" }
            )
            .error()
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

            Self::Execution { errors, context } => {
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
                        error = error.display(context)
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
    pub fn no_context(self, root_dir: String) -> Report {
        match self {
            Self::Execution(errors) => Report::Execution {
                errors,
                context: ErrorContext {
                    root_dir,
                    modules: Default::default(),
                    nodes: Default::default(),
                    types: Default::default(),
                },
            },
        }
    }
}
