use crate::analyzer::context::NodeContext;

use super::{position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Node<T, S, C>(pub T, pub Range<S>, pub C)
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement;

impl<T, S, C> Node<T, S, C>
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
{
    pub fn new(x: T, range: Range<S>, context: C) -> Self {
        Self(x, range, context)
    }

    pub fn value(&self) -> &T {
        &self.0
    }

    pub fn range(&self) -> &Range<S> {
        &self.1
    }

    pub fn context(&self) -> &C {
        &self.2
    }

    pub fn with_context<R>(self, context: R) -> Node<T, S, R> {
        Node(self.0, self.1, context)
    }

    pub fn map_value<R>(self, f: impl FnOnce(T) -> R) -> Node<R, S, C> {
        Node(f(self.0), self.1, self.2)
    }

    pub fn map_range(self, f: impl FnOnce(Range<S>) -> Range<S>) -> Node<T, S, C> {
        Node(self.0, f(self.1), self.2)
    }
}

impl<T, S> Node<T, S, ()>
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: T, range: Range<S>) -> Self {
        Self(x, range, ())
    }
}

impl<T, S> Node<T, S, NodeContext>
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
{
    pub fn id(&self) -> &usize {
        self.2.id()
    }
}

impl<T, S, C> Clone for Node<T, S, C>
where
    T: Clone,
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
    C: Clone,
{
    fn clone(&self) -> Self {
        Self(self.0.clone(), self.1.clone(), self.2.clone())
    }
}
