use super::{ExecutionError, Failure, IntoErrors};
use std::{
    cell::{Ref, RefCell, RefMut},
    rc::Rc,
};

struct ReporterState {
    fail_fast: bool,
    errors: Vec<ExecutionError>,
}

impl ReporterState {
    pub fn new(fail_fast: bool) -> Self {
        Self {
            fail_fast,
            errors: Default::default(),
        }
    }
}

pub struct Reporter {
    state: Rc<RefCell<ReporterState>>,
}

impl Reporter {
    pub fn new(fail_fast: bool) -> Self {
        Self {
            state: Rc::new(RefCell::new(ReporterState::new(fail_fast))),
        }
    }

    fn state(&self) -> Ref<ReporterState> {
        (*self.state).borrow()
    }

    fn state_mut(&self) -> RefMut<ReporterState> {
        (*self.state).borrow_mut()
    }

    fn should_fail_early(&self) -> bool {
        self.state().fail_fast && self.should_fail()
    }

    fn should_fail(&self) -> bool {
        !self.state().errors.is_empty()
    }

    /// add errors to state
    fn extend<T>(&mut self, x: T)
    where
        T: IntoErrors,
    {
        self.state_mut().errors.extend(x.into_errors());
    }

    fn to_failure(&self) -> Failure {
        let state = (*self.state).borrow();

        Failure::Execution(state.errors.clone())
    }

    /// report an error
    /// returns an `Err` if configured to fail fast otherwise `Ok`
    pub fn raise<T>(&mut self, x: T) -> crate::Internal<()>
    where
        T: IntoErrors,
    {
        self.extend(x);

        if self.should_fail_early() {
            Err(Box::new(self.to_failure()))
        } else {
            Ok(())
        }
    }

    /// returns an `Err` if any errors have been reported otherwise `Ok`
    pub fn flush(&self) -> crate::Internal<()> {
        if self.should_fail() {
            Err(Box::new(self.to_failure()))
        } else {
            Ok(())
        }
    }

    /// report an error and return the report
    pub fn fail<T>(&mut self, x: T) -> Failure
    where
        T: IntoErrors,
    {
        self.extend(x);
        self.to_failure()
    }
}

impl Clone for Reporter {
    fn clone(&self) -> Self {
        Self {
            state: Rc::clone(&self.state),
        }
    }
}
