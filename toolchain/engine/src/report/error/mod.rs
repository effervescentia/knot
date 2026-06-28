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
use kore::{
    color::{Colorize, Highlight},
    format::{indented, Indented},
};
use std::{collections::HashMap, fmt::Write, path::PathBuf};

pub trait Display<'a> {
    type Context;

    fn display(&'a self, context: Self::Context) -> ErrorDisplay<'a>;
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct ErrorContext {
    pub root_dir: String,
    pub modules: HashMap<lang::ModuleId, (PathBuf, String)>,
    pub nodes: HashMap<lang::CanonicalId, lang::Range>,
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

impl std::fmt::Display for ErrorDisplay<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{title} {code}\n\n{description}",
            title = self.title.error().bold().underline(),
            code = format!("(E#{})", self.code.0).subtle(),
            description = Indented(&self.description)
        )?;

        if let Some(code_frame) = &self.code_frame {
            write!(indented(f), "\n\n{code_frame}")?;
        }

        for (description, code_frame) in &self.references {
            write!(indented(f), "\n\n{description}\n\n{code_frame}")?;
        }

        if let Some(suggestion) = &self.suggestion {
            write!(
                indented(f),
                "\n\n{}\n\n{suggestion}",
                "How to Fix".success().bold().underline()
            )?;
        }

        for (title, summary) in &self.examples {
            write!(
                indented(f),
                "\n\n{label} {title}\n\n{summary}",
                label = "Example:".highlight(),
                title = title.highlight().bold().underline()
            )?;
        }

        Ok(())
    }
}

#[derive(Default)]
pub struct ErrorDisplayBuilder<'a> {
    code: Option<ErrorCode>,
    title: Option<String>,
    description: Option<String>,
    code_frame: Option<CodeFrame<'a>>,
    references: Vec<(String, CodeFrame<'a>)>,
    suggestion: Option<String>,
    examples: Vec<(String, String)>,
}

impl<'a> ErrorDisplayBuilder<'a> {
    pub const fn code(mut self, code: ErrorCode) -> Self {
        self.code = Some(code);
        self
    }

    pub fn title<T>(mut self, title: T) -> Self
    where
        T: AsRef<str>,
    {
        self.title = Some(title.as_ref().to_owned());
        self
    }

    pub fn description<T>(mut self, description: T) -> Self
    where
        T: AsRef<str>,
    {
        self.description = Some(description.as_ref().to_owned());
        self
    }

    pub const fn code_frame(mut self, code_frame: CodeFrame<'a>) -> Self {
        self.code_frame = Some(code_frame);
        self
    }

    pub fn reference<T>(mut self, description: T, code_frame: CodeFrame<'a>) -> Self
    where
        T: AsRef<str>,
    {
        self.references
            .push((description.as_ref().to_owned(), code_frame));
        self
    }

    pub fn suggestion<T>(mut self, suggestion: T) -> Self
    where
        T: AsRef<str>,
    {
        self.suggestion = Some(suggestion.as_ref().to_owned());
        self
    }

    pub fn example(mut self, example: (String, String)) -> Self {
        self.examples.push(example);
        self
    }

    pub fn build(self) -> ErrorDisplay<'a> {
        ErrorDisplay {
            code: self.code.expect("missing error code"),
            title: self.title.expect("missing error title"),
            description: self.description.expect("missing error description"),
            code_frame: self.code_frame,
            references: self.references,
            suggestion: self.suggestion,
            examples: self.examples,
        }
    }
}
