use crate::analyzer::context::NodeContext;

use super::range::Range;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Node<T, C>(pub T, pub Range, pub C);

impl<T, C> Node<T, C> {
    pub fn new(x: T, range: Range, context: C) -> Self {
        Self(x, range, context)
    }

    pub fn value(&self) -> &T {
        &self.0
    }

    pub fn range(&self) -> &Range {
        &self.1
    }

    pub fn context(&self) -> &C {
        &self.2
    }

    pub fn with_context<R>(self, context: R) -> Node<T, R> {
        Node(self.0, self.1, context)
    }

    pub fn map_value<R>(self, f: impl FnOnce(T) -> R) -> Node<R, C> {
        Node(f(self.0), self.1, self.2)
    }

    pub fn map_range(self, f: impl FnOnce(Range) -> Range) -> Node<T, C> {
        Node(self.0, f(self.1), self.2)
    }
}

impl<T> Node<T, ()> {
    pub fn raw(x: T, range: Range) -> Self {
        Self(x, range, ())
    }
}

impl<T> Node<T, NodeContext> {
    pub fn id(&self) -> &usize {
        self.2.id()
    }
}

impl<T, C> Clone for Node<T, C>
where
    T: Clone,
    C: Clone,
{
    fn clone(&self) -> Self {
        Self(self.0.clone(), self.1.clone(), self.2.clone())
    }
}
