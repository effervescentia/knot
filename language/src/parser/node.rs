use combine::Stream;

use super::{
    position::Decrement,
    range::{Range, Ranged},
};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Node<T, S, C>(pub T, pub Range<S>, pub C)
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement;

impl<T, S> Node<T, S, ()>
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: T, range: Range<S>) -> Self {
        Self(x, range, ())
    }
}

impl<T, S> Ranged<T, S> for Node<T, S, ()>
where
    S: Stream<Token = char>,
    S::Position: Copy + Debug + Decrement,
{
    fn value(self) -> T {
        self.0
    }

    fn range(&self) -> &Range<S> {
        &self.1
    }
}
