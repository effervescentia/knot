use engine::engine2::Logger as _;
use kore::color::Highlight;
use log::{debug, error, info, trace, warn};

pub struct Logger;

impl Logger {
    pub fn report_parsed(&self, count: usize) {
        self.debug(format_args!(
            "\u{1f440} parsed {} matching module(s)",
            count.to_string().focus()
        ));
    }
}

impl engine::engine2::Logger for Logger {
    fn error(&self, args: std::fmt::Arguments) {
        error!("{}", args);
    }

    fn warn(&self, args: std::fmt::Arguments) {
        warn!("{}", args);
    }

    fn info(&self, args: std::fmt::Arguments) {
        info!("{}", args);
    }

    fn debug(&self, args: std::fmt::Arguments) {
        debug!("{}", args);
    }

    fn trace(&self, args: std::fmt::Arguments) {
        trace!("{}", args);
    }
}
