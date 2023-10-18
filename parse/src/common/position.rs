use combine::stream::position::SourcePosition;

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
