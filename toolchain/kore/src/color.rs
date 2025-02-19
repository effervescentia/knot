pub use colored::{ColoredString, Colorize};
use std::env;

fn is_truecolor() -> bool {
    matches!(env::var("COLORTERM"), Ok(val) if val == "truecolor" || val == "24bit")
}

pub trait Highlight: Colorize + Sized {
    fn subtle(self) -> ColoredString {
        if is_truecolor() {
            self.truecolor(115, 115, 115)
        } else {
            self.black()
        }
    }

    fn error(self) -> ColoredString {
        if is_truecolor() {
            self.truecolor(252, 85, 85)
        } else {
            self.red()
        }
    }

    fn success(self) -> ColoredString {
        if is_truecolor() {
            self.truecolor(76, 240, 84)
        } else {
            self.green()
        }
    }

    fn highlight(self) -> ColoredString {
        if is_truecolor() {
            self.truecolor(115, 200, 252)
        } else {
            self.cyan()
        }
    }

    fn focus(self) -> ColoredString {
        self.bright_white().bold()
    }
}

impl Highlight for &str {}

pub trait ClearIf {
    fn clear_if(self, condition: bool) -> Self;
}

impl ClearIf for ColoredString {
    fn clear_if(self, condition: bool) -> Self {
        if condition {
            self.clear()
        } else {
            self
        }
    }
}
