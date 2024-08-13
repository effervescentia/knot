mod expression;
mod statement;

use crate::javascript::JavaScript;
use kore::format::SuffixEach;
use std::fmt::Display;

impl Display for JavaScript {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{statements}", statements = SuffixEach("\n", &self.0))
    }
}
