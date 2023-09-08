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
