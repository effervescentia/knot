use crate::{walk::Walk, Range};
use std::fmt::{Debug, Display};

#[derive(Clone, Debug, PartialEq)]
pub struct Node<Value, Meta>(pub Value, pub Range, pub Meta);

impl<Value, Meta> Node<Value, Meta> {
    pub const fn typed(v: Value, m: Meta) -> Self {
        Self(v, Range::nil(), m)
    }

    pub const fn value(&self) -> &Value {
        &self.0
    }

    pub const fn range(&self) -> &Range {
        &self.1
    }

    pub const fn meta(&self) -> &Meta {
        &self.2
    }

    pub fn map_value<T, F>(self, f: F) -> Node<T, Meta>
    where
        F: FnOnce(Value) -> T,
    {
        Node(f(self.0), self.1, self.2)
    }

    pub fn map_range<F>(self, f: F) -> Self
    where
        F: FnOnce(Range) -> Range,
    {
        Self(self.0, f(self.1), self.2)
    }

    pub fn with_meta<T>(self, meta: T) -> Node<Value, T> {
        Node(self.0, self.1, meta)
    }

    #[cfg(feature = "test")]
    pub const fn mock(v: Value, m: Meta) -> Self {
        Self(v, Range::nil(), m)
    }
}

impl<Value> Node<Value, ()> {
    pub const fn raw(value: Value, range: Range) -> Self {
        Self(value, range, ())
    }
}

impl<Visitor, Value, Meta> Walk<Visitor> for Node<Value, Meta>
where
    (Value, (Range, Meta)): Walk<Visitor>,
{
    type Output = <(Value, (Range, Meta)) as Walk<Visitor>>::Output;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        (self.0, (self.1, self.2)).walk(v)
    }
}

impl<Value, Meta> Display for Node<Value, Meta>
where
    Value: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        self.value().fmt(f)
    }
}
