use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind, Type,
};
use lang::ast::{BinaryOperator, Expression, Primitive, UnaryOperator};

impl ToWeak for Expression<usize, usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Primitive(x) => (
                RefKind::Value,
                Weak::Type(match x {
                    Primitive::Nil => Type::Nil,
                    Primitive::Boolean(..) => Type::Boolean,
                    Primitive::Integer(..) => Type::Integer,
                    Primitive::Float(..) => Type::Float,
                    Primitive::String(..) => Type::String,
                }),
            ),

            Self::Identifier(..) => (RefKind::Value, Weak::Infer),

            Self::Group(id) => (RefKind::Value, Weak::Inherit(**id)),

            Self::Closure(xs) => (RefKind::Value, {
                match xs.last() {
                    Some(id) => Weak::Inherit(*id),
                    None => Weak::Type(Type::Nil),
                }
            }),

            Self::UnaryOperation(op, id) => (
                RefKind::Value,
                match op {
                    UnaryOperator::Not => Weak::Type(Type::Boolean),

                    _ => Weak::Inherit(**id),
                },
            ),

            Self::BinaryOperation(op, ..) => (
                RefKind::Value,
                match op {
                    BinaryOperator::Equal
                    | BinaryOperator::NotEqual
                    | BinaryOperator::And
                    | BinaryOperator::Or
                    | BinaryOperator::LessThan
                    | BinaryOperator::LessThanOrEqual
                    | BinaryOperator::GreaterThan
                    | BinaryOperator::GreaterThanOrEqual => Weak::Type(Type::Boolean),

                    BinaryOperator::Divide | BinaryOperator::Exponent => Weak::Type(Type::Float),

                    BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => {
                        Weak::Infer
                    }
                },
            ),

            Self::DotAccess(..) => (RefKind::Value, Weak::Infer),

            Self::FunctionCall(..) => (RefKind::Value, Weak::Infer),

            Self::Style(..) => (RefKind::Value, Weak::Type(Type::Style)),

            Self::KSX(..) => (RefKind::Value, Weak::Type(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind, Type,
    };
    use kore::str;
    use lang::ast::{BinaryOperator, Expression, Primitive, UnaryOperator};

    #[test]
    fn primitive() {
        assert_eq!(
            Expression::Primitive(Primitive::Nil).to_weak(),
            (RefKind::Value, Weak::Type(Type::Nil))
        );
        assert_eq!(
            Expression::Primitive(Primitive::Boolean(true)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
        assert_eq!(
            Expression::Primitive(Primitive::Integer(123)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Integer))
        );
        assert_eq!(
            Expression::Primitive(Primitive::Float(123.45, 5)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Float))
        );
        assert_eq!(
            Expression::Primitive(Primitive::String(str!("foo"))).to_weak(),
            (RefKind::Value, Weak::Type(Type::String))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            Expression::Identifier(str!("foo")).to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            Expression::Group(Box::new(0)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            Expression::Closure(vec![0, 1]).to_weak(),
            (RefKind::Value, Weak::Inherit(1))
        );
    }

    #[test]
    fn closure_empty() {
        assert_eq!(
            Expression::Closure(vec![]).to_weak(),
            (RefKind::Value, Weak::Type(Type::Nil))
        );
    }

    #[test]
    fn unary_not() {
        assert_eq!(
            Expression::UnaryOperation(UnaryOperator::Not, Box::new(0)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn unary_absolute() {
        assert_eq!(
            Expression::UnaryOperation(UnaryOperator::Absolute, Box::new(0)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn unary_negate() {
        assert_eq!(
            Expression::UnaryOperation(UnaryOperator::Negate, Box::new(0)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn binary_equal() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Equal, Box::new(0), Box::new(1)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_not_equal() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::NotEqual, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_and() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::And, Box::new(0), Box::new(1)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_or() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Or, Box::new(0), Box::new(1)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_less_than() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::LessThan, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_less_than_or_equal() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::LessThanOrEqual, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_greater_than() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::GreaterThan, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_greater_than_or_equal() {
        assert_eq!(
            Expression::BinaryOperation(
                BinaryOperator::GreaterThanOrEqual,
                Box::new(0),
                Box::new(1)
            )
            .to_weak(),
            (RefKind::Value, Weak::Type(Type::Boolean))
        );
    }

    #[test]
    fn binary_divide() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Divide, Box::new(0), Box::new(1)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Float))
        );
    }

    #[test]
    fn binary_exponent() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Exponent, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Type(Type::Float))
        );
    }

    #[test]
    fn binary_add() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Add, Box::new(0), Box::new(1)).to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn binary_subtract() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Subtract, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn binary_multiply() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Multiply, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            Expression::DotAccess(Box::new(0), str!("foo")).to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            Expression::FunctionCall(Box::new(0), vec![1]).to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            Expression::Style(vec![(str!("foo"), 0)]).to_weak(),
            (RefKind::Value, Weak::Type(Type::Style))
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            Expression::KSX(Box::new(0)).to_weak(),
            (RefKind::Value, Weak::Type(Type::Element))
        );
    }
}
