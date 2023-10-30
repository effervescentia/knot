use crate::{Error, Result};
use std::{
    cell::{Ref, RefCell, RefMut},
    rc::Rc,
};

struct ReporterState {
    fail_fast: bool,
    errors: Vec<Error>,
}

impl ReporterState {
    pub const fn new(fail_fast: bool) -> Self {
        Self {
            fail_fast,
            errors: vec![],
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

    fn errors(&self) -> Vec<Error> {
        self.state_mut().errors.clone()
    }

    pub fn report<I>(&mut self, errors: I)
    where
        I: IntoIterator<Item = Error>,
    {
        self.state_mut().errors.extend(errors);
    }

    pub fn raise<I>(&mut self, errors: I) -> Result<()>
    where
        I: IntoIterator<Item = Error>,
    {
        self.report(errors);
        self.catch()
    }

    pub fn should_fail_early(&self) -> bool {
        self.state().fail_fast && self.should_fail()
    }

    pub fn should_fail(&self) -> bool {
        !self.state().errors.is_empty()
    }

    pub fn catch(&self) -> Result<()> {
        if self.should_fail() {
            Err(self.errors())
        } else {
            Ok(())
        }
    }
}

impl Clone for Reporter {
    fn clone(&self) -> Self {
        Self {
            state: Rc::clone(&self.state),
        }
    }
}
