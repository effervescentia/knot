use combine::{stream::PointerOffset, Stream, StreamOnce};
use std::fmt::Debug;

#[derive(Copy, Debug, PartialEq)]
pub struct Range<T>(pub T::Position, pub T::Position)
where
    T: StreamOnce,
    T::Position: Copy + Debug;

impl<T> Range<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
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

    pub fn concat(&self, range: &Self) -> Self {
        Self(self.0.min(range.0), self.1.max(range.1))
    }
}

impl Range<&str> {
    pub fn str(start: usize, end: usize) -> Self {
        Range(PointerOffset::new(start), PointerOffset::new(end))
    }
}

impl<T> Clone for Range<T>
where
    T: StreamOnce,
    T::Position: Copy + Debug,
{
    fn clone(&self) -> Self {
        Self(self.0.clone(), self.1.clone())
    }
}
