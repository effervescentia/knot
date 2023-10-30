use parse::Range;

use crate::Link;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CodeFrame {
    link: Link,
    range: Range,
}

impl CodeFrame {
    pub const fn new(link: Link, range: Range) -> Self {
        Self { link, range }
    }

    pub const fn link(&self) -> &Link {
        &self.link
    }
}
