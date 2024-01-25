use crate::{
    ast::walk::{Span, ToSpan},
    Range,
};
use std::fmt::{Debug, Display, Formatter};

#[derive(Debug, PartialEq)]
pub struct Node<Value, Meta>(pub Value, pub Range, pub Meta);

impl<Value, Meta> Node<Value, Meta> {
    pub const fn value(&self) -> &Value {
        &self.0
    }

    pub const fn range(&self) -> &Range {
        &self.1
    }

    pub const fn meta(&self) -> &Meta {
        &self.2
    }

    pub fn map_value<T>(self, f: impl FnOnce(Value) -> T) -> Node<T, Meta> {
        Node(f(self.0), self.1, self.2)
    }

    pub fn map_range(self, f: impl FnOnce(Range) -> Range) -> Self {
        Self(self.0, f(self.1), self.2)
    }

    pub fn with_meta<T>(self, meta: T) -> Node<Value, T> {
        Node(self.0, self.1, meta)
    }
}

impl<Value> Node<Value, ()> {
    pub const fn raw(value: Value, range: Range) -> Self {
        Self(value, range, ())
    }
}

impl<Value, Meta> ToSpan<Value> for Node<Value, Meta> {
    fn to_span(self) -> Span<Value> {
        Span(self.0, self.1)
    }
}

impl<Value, Meta> Display for Node<Value, Meta>
where
    Value: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.value().fmt(f)
    }
}
