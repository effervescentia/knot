use kore::color::Highlight;
use std::fmt::Display;

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
