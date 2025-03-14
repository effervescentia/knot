use engine::engine2::{Analyzed, Linked, Logger as _, Parsed};
use kore::color::Highlight;
use log::{debug, error, info, trace, warn};

pub struct Logger;

impl Logger {
    pub fn report_parsed(&self, Parsed(ids): &Parsed) {
        self.debug(format_args!(
            "\u{1f440} parsed {} matching module(s)",
            ids.len().to_string().focus()
        ));
    }

    pub fn report_linked(&self, Linked(ids): &Linked) {
        self.debug(format_args!(
            "\u{1f440} linked {} module(s)",
            ids.len().to_string().focus()
        ));
    }

    pub fn report_analyzed(&self, Analyzed(ids): &Analyzed) {
        self.debug(format_args!(
            "\u{1f440} analyzed {} module(s)",
            ids.len().to_string().focus()
        ));
    }
}

impl engine::engine2::Logger for Logger {
    fn error(&self, args: std::fmt::Arguments) {
        error!("{args}");
    }

    fn warn(&self, args: std::fmt::Arguments) {
        warn!("{args}");
    }

    fn info(&self, args: std::fmt::Arguments) {
        info!("{args}");
    }

    fn debug(&self, args: std::fmt::Arguments) {
        debug!("{args}");
    }

    fn trace(&self, args: std::fmt::Arguments) {
        trace!("{args}");
    }
}
