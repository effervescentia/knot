mod code_frame;
mod error;
mod errors;
mod reporter;

pub use code_frame::CodeFrame;
pub use error::{ConfigurationError, ExecutionError};
pub use errors::Errors;
use kore::color::{Colorize, Highlight};
use lang::{CanonicalId, NamespaceId};
pub use reporter::Reporter;
use std::{collections::HashMap, fmt::Display};

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Report {
    Configuration(ConfigurationError),
    Execution {
        modules: HashMap<NamespaceId, String>,
        nodes: HashMap<CanonicalId, String>,
        errors: Vec<ExecutionError>,
    },
}

impl Report {
    #[cfg(feature = "test")]
    pub fn exec_errors(&self) -> Option<&Vec<ExecutionError>> {
        match self {
            Report::Execution { errors, .. } => Some(errors),
            Report::Configuration(_) => None,
        }
    }
}

impl Display for Report {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Configuration(error) => {
                // let error_count = errors.len();
                let error_count_bumper =
                    format!("finished with {} error(s)", 1.to_string().bold()).error();

                writeln!(f, "{}\n", error_count_bumper)?;

                // for (index, error) in errors.iter().enumerate() {
                writeln!(f, "{index} {error}\n", index = format!("{})", 1).error())?;
                // }

                writeln!(f, "{}\n", error_count_bumper)
            }

            Self::Execution { errors, .. } => {
                let error_count = errors.len();
                let error_count_bumper =
                    format!("finished with {} error(s)", error_count.to_string().bold()).error();

                writeln!(f, "{}\n", error_count_bumper)?;

                for (index, error) in errors.iter().enumerate() {
                    writeln!(
                        f,
                        "{index} {error}\n",
                        index = format!("{})", index + 1).error()
                    )?;
                }

                writeln!(f, "{}\n", error_count_bumper)
            }
        }
    }
}
