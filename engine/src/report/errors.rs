use super::{ExecutionError, InternalReport};
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

impl Errors for crate::InternalResult<()> {
    fn errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        match self {
            Ok(()) => vec![],
            Err(InternalReport::Execution(errors)) => errors,
        }
        .errors()
    }
}
