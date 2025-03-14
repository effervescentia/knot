use std::fmt::Arguments;

pub trait Logger {
    fn error(&self, args: Arguments);
    fn warn(&self, args: Arguments);
    fn info(&self, args: Arguments);
    fn debug(&self, args: Arguments);
    fn trace(&self, args: Arguments);
}

pub struct NoopLogger;

impl Logger for NoopLogger {
    fn error(&self, _args: Arguments) {}
    fn warn(&self, _args: Arguments) {}
    fn info(&self, _args: Arguments) {}
    fn debug(&self, _args: Arguments) {}
    fn trace(&self, _args: Arguments) {}
}

#[cfg(test)]
#[derive(Default)]
pub struct MemoryLogger(std::cell::RefCell<Vec<(String, String)>>);

#[cfg(test)]
impl MemoryLogger {
    pub fn assert(&self, logs: &[(String, String)]) {
        kore::assert_eq!(self.0.take(), logs);
    }
}

#[cfg(test)]
impl Logger for MemoryLogger {
    fn error(&self, args: Arguments) {
        self.0
            .borrow_mut()
            .push((kore::str!("error"), args.to_string()));
    }

    fn warn(&self, args: Arguments) {
        self.0
            .borrow_mut()
            .push((kore::str!("warn"), args.to_string()));
    }

    fn info(&self, args: Arguments) {
        self.0
            .borrow_mut()
            .push((kore::str!("info"), args.to_string()));
    }

    fn debug(&self, args: Arguments) {
        self.0
            .borrow_mut()
            .push((kore::str!("debug"), args.to_string()));
    }

    fn trace(&self, args: Arguments) {
        self.0
            .borrow_mut()
            .push((kore::str!("trace"), args.to_string()));
    }
}
