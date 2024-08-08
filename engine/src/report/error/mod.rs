mod analysis;
mod code;
mod configuration;
mod environment;
mod execution;

use super::CodeFrame;
use code::ErrorCode;
pub use configuration::ConfigurationError;
pub use environment::EnvironmentError;
pub use execution::ExecutionError;
use kore::color::{Colorize, Highlight};

pub trait Display<'a> {
    type Context;

    fn display(&'a self, context: Self::Context) -> ErrorDisplay<'a>;
}

pub struct ErrorDisplay<'a> {
    code: ErrorCode,
    title: &'a str,
    description: String,
    code_frame: Option<CodeFrame<'a>>,
}

impl<'a> std::fmt::Display for ErrorDisplay<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{title} {code}\n\n{description}",
            title = self.title.error().bold(),
            code = format!("(E#{})", self.code.0).subtle(),
            description = self.description
        )?;

        if let Some(code_frame) = &self.code_frame {
            write!(f, "\n\n{}", code_frame)?;
        }

        Ok(())
    }
}
