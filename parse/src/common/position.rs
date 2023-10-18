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

// #[derive(Clone, PartialEq, Eq, PartialOrd, Ord)]
// pub struct Position(SourcePosition);

// impl lang::Position for Position {
//     fn line(&self) -> i32 {
//         self.0.line
//     }

//     fn column(&self) -> i32 {
//         self.0.column
//     }

//     fn decrement(&self) -> Self {
//         Self(SourcePosition {
//             line: self.line(),
//             column: self.column() - 1,
//         })
//     }
// }

// impl Into<Position> for SourcePosition {
//     fn into(self) -> Position {
//         Position(self)
//     }
// }
