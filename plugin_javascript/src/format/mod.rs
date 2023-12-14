mod expression;
mod statement;

use crate::javascript::JavaScript;
use kore::format::SuffixEach;
use std::fmt::{Display, Formatter};

impl Display for JavaScript {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{statements}", statements = SuffixEach("\n", &self.0))
    }
}
