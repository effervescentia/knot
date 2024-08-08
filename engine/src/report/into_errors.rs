use super::{ExecutionError, Failure};
use std::iter::once;

pub trait IntoErrors {
    fn into_errors(self) -> Box<dyn Iterator<Item = ExecutionError>>;
}

impl IntoErrors for ExecutionError {
    fn into_errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        Box::new(once(self))
    }
}

impl IntoErrors for Vec<ExecutionError> {
    fn into_errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        Box::new(self.into_iter())
    }
}

impl IntoErrors for crate::Internal<()> {
    fn into_errors(self) -> Box<dyn Iterator<Item = ExecutionError>> {
        match self {
            Ok(()) => vec![],
            Err(Failure::Execution(errors)) => errors,
        }
        .into_errors()
    }
}
