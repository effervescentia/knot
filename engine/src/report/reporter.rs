use super::{Errors, ExecutionError};
use crate::{Report, Result};
use lang::{CanonicalId, NamespaceId};
use std::{
    cell::{Ref, RefCell, RefMut},
    collections::HashMap,
    rc::Rc,
};

struct ReporterState {
    fail_fast: bool,
    modules: HashMap<NamespaceId, String>,
    nodes: HashMap<CanonicalId, String>,
    errors: Vec<ExecutionError>,
}

impl ReporterState {
    pub fn new(fail_fast: bool) -> Self {
        Self {
            fail_fast,
            modules: Default::default(),
            nodes: Default::default(),
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
        T: Errors,
    {
        self.state_mut().errors.extend(x.errors())
    }

    fn to_report(&self) -> Report {
        let state = (*self.state).borrow();

        Report::Execution {
            modules: state.modules.clone(),
            nodes: state.nodes.clone(),
            errors: state.errors.clone(),
        }
    }

    /// report an error
    /// returns an `Err` if configured to fail fast otherwise `Ok`
    pub fn raise<T>(&mut self, x: T) -> Result<()>
    where
        T: Errors,
    {
        self.extend(x);

        if self.should_fail_early() {
            Err(self.to_report())
        } else {
            Ok(())
        }
    }

    /// returns an `Err` if any errors have been reported otherwise `Ok`
    pub fn flush(&self) -> Result<()> {
        if self.should_fail() {
            Err(self.to_report())
        } else {
            Ok(())
        }
    }

    /// report an error and return the report
    pub fn fail<T>(&mut self, x: T) -> Report
    where
        T: Errors,
    {
        self.extend(x);
        self.to_report()
    }
}

impl Clone for Reporter {
    fn clone(&self) -> Self {
        Self {
            state: Rc::clone(&self.state),
        }
    }
}
