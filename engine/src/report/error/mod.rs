mod analysis;
mod code;
mod configuration;
mod environment;
mod execution;

use std::collections::HashMap;

use super::CodeFrame;
use code::ErrorCode;
pub use configuration::ConfigurationError;
pub use environment::EnvironmentError;
pub use execution::ExecutionError;
use kore::{
    color::{Colorize, Highlight},
    format::Indented,
};

pub trait Display<'a> {
    type Context;

    fn display(&'a self, context: Self::Context) -> ErrorDisplay<'a>;
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct ErrorContext {
    pub root_dir: String,
    pub modules: HashMap<lang::NamespaceId, (crate::Link, String)>,
    pub nodes: HashMap<lang::CanonicalId, lang::Range>,
    pub types: HashMap<lang::CanonicalId, ()>,
}

pub struct ErrorDisplay<'a> {
    code: ErrorCode,
    title: String,
    description: String,
    code_frame: Option<CodeFrame<'a>>,
    references: Vec<(String, CodeFrame<'a>)>,
    suggestion: Option<String>,
    examples: Vec<(String, String)>,
}

impl<'a> ErrorDisplay<'a> {
    pub fn simple(code: ErrorCode, title: &'a str, description: String) -> Self {
        Self {
            code,
            title: title.to_owned(),
            description,
            code_frame: None,
            references: vec![],
            suggestion: None,
            examples: vec![],
        }
    }
}

impl<'a> std::fmt::Display for ErrorDisplay<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{title} {code}\n\n{description}",
            title = self.title.error().bold().underline(),
            code = format!("(E#{})", self.code.0).subtle(),
            description = Indented(&self.description)
        )?;

        if let Some(code_frame) = &self.code_frame {
            write!(f, "\n\n{}", Indented(code_frame))?;
        }

        if let Some(suggestion) = &self.suggestion {
            write!(
                f,
                "\n\n{}\n\n{}",
                Indented("How to Fix".success().bold().underline()),
                Indented(suggestion)
            )?;
        }

        for (title, summary) in &self.examples {
            write!(
                f,
                "\n\n{}",
                Indented(format!(
                    "{header}\n\n{summary}",
                    header = format!(
                        "{} {}",
                        "Example:".highlight(),
                        title.highlight().bold().underline()
                    ),
                ))
            )?;
        }

        Ok(())
    }
}

pub struct ErrorDisplayBuilder<T>(T);

impl ErrorDisplayBuilder<ErrorCode> {
    pub const fn factory(code: ErrorCode) -> Self {
        Self(code)
    }

    pub fn new(
        code: ErrorCode,
        title: &str,
        description: String,
    ) -> ErrorDisplayBuilder<ErrorDisplay> {
        ErrorDisplayBuilder(ErrorDisplay::simple(code, title, description))
    }
}

impl<'a> ErrorDisplayBuilder<ErrorDisplay<'a>> {
    pub const fn code_frame(mut self, code_frame: CodeFrame<'a>) -> Self {
        self.0.code_frame = Some(code_frame);
        self
    }

    pub fn reference(mut self, title: String, code_frame: CodeFrame<'a>) -> Self {
        self.0.references.push((title, code_frame));
        self
    }

    pub fn suggestion(mut self, suggestion: String) -> Self {
        self.0.suggestion = Some(suggestion);
        self
    }

    pub fn example(mut self, example: (String, String)) -> Self {
        self.0.examples.push(example);
        self
    }

    pub fn build(self) -> ErrorDisplay<'a> {
        self.0
    }
}
