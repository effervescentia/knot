use std::fmt::{Display, Formatter};

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum UnaryOperator {
    Not,
    Absolute,
    Negate,
}

impl Display for UnaryOperator {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Not => write!(f, "!"),
            Self::Absolute => write!(f, "+"),
            Self::Negate => write!(f, "-"),
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum BinaryOperator {
    And,
    Or,

    Equal,
    NotEqual,

    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,

    Add,
    Subtract,
    Multiply,
    Divide,
    Exponent,
}

impl Display for BinaryOperator {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::And => write!(f, "&&"),
            Self::Or => write!(f, "||"),

            Self::Equal => write!(f, "=="),
            Self::NotEqual => write!(f, "!="),

            Self::LessThan => write!(f, "<"),
            Self::LessThanOrEqual => write!(f, "<="),
            Self::GreaterThan => write!(f, ">"),
            Self::GreaterThanOrEqual => write!(f, ">="),

            Self::Add => write!(f, "+"),
            Self::Subtract => write!(f, "-"),
            Self::Multiply => write!(f, "*"),
            Self::Divide => write!(f, "/"),
            Self::Exponent => write!(f, "^"),
        }
    }
}
