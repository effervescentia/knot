use std::{fmt::Display, ops::Add};

#[derive(Clone, Copy, Debug, Default, Eq, Hash, PartialEq, PartialOrd, Ord)]
pub struct Point(pub usize, pub usize);

impl Point {
    pub const fn decrement(&self) -> Self {
        Self(self.0, self.1 - 1)
    }

    pub const fn offset(&self, (offset_row, offset_column): (usize, usize)) -> Self {
        Self(self.0 + offset_row, self.1 + offset_column)
    }
}

impl Display for Point {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}:{}", self.0, self.1)
    }
}

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct Range(pub Point, pub Point);

impl Range {
    pub const fn nil() -> Self {
        Self::new((0, 0), (0, 0))
    }

    pub const fn new(
        (start_row, start_column): (usize, usize),
        (end_row, end_column): (usize, usize),
    ) -> Self {
        Self(Point(start_row, start_column), Point(end_row, end_column))
    }

    pub fn extend(&self, point: &Point) -> Self {
        if point < &self.0 {
            Self(*point, self.1)
        } else if point > &self.1 {
            Self(self.0, *point)
        } else {
            *self
        }
    }

    pub const fn offset(&self, offset: (usize, usize)) -> Self {
        Self(self.0.offset(offset), self.1.offset(offset))
    }
}

impl Add for &Range {
    type Output = Range;

    fn add(self, rhs: Self) -> Self::Output {
        Range(self.0.min(rhs.0), self.1.max(rhs.1))
    }
}
