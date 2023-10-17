use combine::stream::{position::SourcePosition, PointerOffset};

pub trait Decrement {
    fn decrement(self) -> Self;
}

impl Decrement for SourcePosition {
    fn decrement(self) -> Self {
        SourcePosition {
            line: self.line,
            column: self.column - 1,
        }
    }
}

impl Decrement for PointerOffset<str> {
    /// decrementing pointer offset is not supported
    fn decrement(self) -> Self {
        PointerOffset::new(self.0)
    }
}

pub type Point = (i32, i32);

pub trait Position {
    fn line(&self) -> i32;

    fn column(&self) -> i32;

    fn point(&self) -> Point {
        (self.line(), self.column())
    }

    fn decrement(&self) -> Self;
}

impl Position for SourcePosition {
    fn line(&self) -> i32 {
        self.line
    }

    fn column(&self) -> i32 {
        self.column
    }

    fn decrement(&self) -> Self {
        SourcePosition {
            line: self.line,
            column: self.column - 1,
        }
    }
}

impl Position for PointerOffset<str> {
    /// this does not return a real value
    fn line(&self) -> i32 {
        1
    }

    /// this does not return a real value
    fn column(&self) -> i32 {
        1
    }

    /// decrementing pointer offset is not supported
    fn decrement(&self) -> Self {
        self.clone()
    }
}
