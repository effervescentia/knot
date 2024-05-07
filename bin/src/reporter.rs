use crate::config::{Config, ConfigList};
use command::Error;
use kore::color::{Colorize, Highlight};
use std::fmt::Display;

// const ERROR_HEADER: &str = "\u{2554}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2557}
// \u{2551}                    FAILED                    \u{2551}
// \u{255a}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{255d}";

pub enum Phase {
    Configuration,
    Execution,
    Report,
    Result,
}

impl Display for Phase {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        writeln!(
            f,
            "{}\n",
            format!(
                "[ {} ]",
                match self {
                    Self::Configuration => "configuration",
                    Self::Execution => "execution",
                    Self::Report => "report",
                    Self::Result => "result",
                }
            )
            .subtle()
        )
    }
}

pub fn eprint_report(errors: &[Error]) {
    let error_count = errors.len();
    let error_count_bumper =
        format!("finished with {} error(s)", error_count.to_string().bold()).error();

    eprintln!(
        "{}{}\n",
        Phase::Report,
        // ERROR_HEADER.error(),
        error_count_bumper
    );

    for (index, error) in errors.iter().enumerate() {
        eprintln!(
            "{index} {error}\n",
            index = format!("{})", index + 1).error()
        );
    }

    eprintln!("{}\n", error_count_bumper);
}

pub fn eprint_configuration(configs: Vec<(&'static str, Config)>) {
    eprintln!("{}{}", Phase::Configuration, ConfigList::new(configs));
}
