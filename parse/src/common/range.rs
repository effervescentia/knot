use super::position::{Point, Position};
use std::{fmt::Debug, ops::Add};

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct Range(pub Point, pub Point);

impl Range {
    pub fn from<P>(start: &P, end: &P) -> Self
    where
        P: Position,
    {
        Self(start.point(), end.point())
    }

    pub fn include<P>(&self, position: &P) -> Self
    where
        P: Position,
    {
        let point = position.point();

        if point < self.0 {
            Self(point, self.1)
        } else if point > self.1 {
            Self(self.0, point)
        } else {
            *self
        }
    }
}

impl Add for &Range {
    type Output = Range;

    fn add(self, rhs: Self) -> Self::Output {
        Range(self.0.min(rhs.0), self.1.max(rhs.1))
    }
}
