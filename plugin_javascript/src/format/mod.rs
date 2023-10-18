use crate::javascript::JavaScript;
use format::TerminateEach;
use std::fmt::{Display, Formatter};

mod expression;
mod statement;

impl Display for JavaScript {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{statements}", statements = TerminateEach("\n", &self.0))
    }
}
