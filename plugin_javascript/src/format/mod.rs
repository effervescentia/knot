mod expression;
mod statement;

use crate::javascript::JavaScript;
use kore::format::TerminateEach;
use std::fmt::{Display, Formatter};

impl Display for JavaScript {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{statements}", statements = TerminateEach("\n", &self.0))
    }
}
