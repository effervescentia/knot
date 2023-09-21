#[derive(Clone, Copy, Debug, PartialEq)]
pub enum UnaryOperator {
    Not,
    Absolute,
    Negate,
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
