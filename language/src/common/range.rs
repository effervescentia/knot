use crate::{common::position::Decrement, parser::CharStream};
use combine::{stream::position::SourcePosition, Stream, StreamOnce};
use std::{fmt::Debug, ops::Add};

pub trait Ranged<T, S>
where
    S: StreamOnce,
    S::Position: Copy + Debug + Decrement,
{
    fn value(self) -> T;

    fn range(&self) -> &Range<S>;
}

#[derive(Copy, Debug, PartialEq)]
pub struct Range<T>(pub T::Position, pub T::Position)
where
    T: StreamOnce,
    T::Position: Copy + Debug + Decrement;

impl<T> Range<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn include(&self, position: T::Position) -> Self {
        if position < self.0 {
            Self(position, self.1)
        } else if position > self.1 {
            Self(self.0, position)
        } else {
            self.clone()
        }
    }
}

impl<T> Add for &Range<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Output = Range<T>;

    fn add(self, rhs: Self) -> Self::Output {
        Range(self.0.min(rhs.0), self.1.max(rhs.1))
    }
}

impl<'a> Range<CharStream<'a>> {
    pub const fn chars(
        (start_line, start_column): (i32, i32),
        (end_line, end_column): (i32, i32),
    ) -> Self {
        Range(
            SourcePosition {
                line: start_line,
                column: start_column,
            },
            SourcePosition {
                line: end_line,
                column: end_column,
            },
        )
    }
}

impl<T> Clone for Range<T>
where
    T: StreamOnce,
    T::Position: Copy + Debug + Decrement,
{
    fn clone(&self) -> Self {
        Self(self.0.clone(), self.1.clone())
    }
}
