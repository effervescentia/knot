use crate::Link;
use lang::Range;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CodeFrame<'a> {
    link: &'a Link,
    range: Range,
    source: &'a str,
}

impl<'a> CodeFrame<'a> {
    pub const fn new(link: &'a Link, range: Range, source: &'a str) -> Self {
        Self {
            link,
            range,
            source,
        }
    }

    pub const fn link(&self) -> &Link {
        &self.link
    }
}
