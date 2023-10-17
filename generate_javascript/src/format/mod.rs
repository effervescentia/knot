use std::fmt::{Display, Formatter};

use lang::formatter::TerminateEach;

use crate::javascript::JavaScript;

mod expression;
mod statement;

impl Display for JavaScript {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{statements}", statements = TerminateEach("\n", &self.0))
    }
}
