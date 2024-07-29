use super::ExecutionError;
use crate::Report;
use kore::invariant;
use std::iter::once;

pub trait Errors {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>>;
}

impl Errors for ExecutionError {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        Box::new(once(self))
    }
}

impl Errors for Vec<ExecutionError> {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        Box::new(self.into_iter())
    }
}

impl Errors for crate::Result<()> {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        match self {
            Ok(()) => vec![],
            Err(Report::Configuration(_)) => {
                invariant!("configuration errors should be reported immediately")
            }
            Err(Report::Execution { errors, .. }) => errors,
        }
        .errors()
    }
}

impl Errors for crate::InnerResult<()> {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        match self {
            Ok(()) => vec![],
            Err(errors) => errors,
        }
        .errors()
    }
}
