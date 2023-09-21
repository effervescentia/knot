use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Type, Weak, WeakRef},
    ast::{
        expression::{Expression, Primitive},
        operator::{BinaryOperator, UnaryOperator},
    },
};

impl ToWeak for Expression<usize, usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Expression::Primitive(x) => (
                RefKind::Value,
                Weak::Type(match x {
                    Primitive::Nil => Type::Nil,
                    Primitive::Boolean(..) => Type::Boolean,
                    Primitive::Integer(..) => Type::Integer,
                    Primitive::Float(..) => Type::Float,
                    Primitive::String(..) => Type::String,
                }),
            ),

            Expression::Identifier(..) => (RefKind::Value, Weak::Unknown),

            Expression::Group(id) => (RefKind::Value, Weak::Inherit(**id)),

            Expression::Closure(xs) => (RefKind::Value, {
                match xs.last() {
                    Some(id) => Weak::Inherit(*id),
                    None => Weak::Type(Type::Nil),
                }
            }),

            Expression::UnaryOperation(op, id) => (
                RefKind::Value,
                match op {
                    UnaryOperator::Not => Weak::Type(Type::Boolean),

                    _ => Weak::Inherit(**id),
                },
            ),

            Expression::BinaryOperation(op, ..) => (
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
                        Weak::Unknown
                    }
                },
            ),

            Expression::DotAccess(..) => (RefKind::Value, Weak::Unknown),

            Expression::FunctionCall(..) => (RefKind::Value, Weak::Unknown),

            Expression::Style(..) => (RefKind::Value, Weak::Type(Type::Style)),

            Expression::KSX(..) => (RefKind::Value, Weak::Type(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Type, Weak},
        ast::{
            expression::{Expression, Primitive},
            operator::{BinaryOperator, UnaryOperator},
        },
    };

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
            Expression::Primitive(Primitive::String(String::from("foo"))).to_weak(),
            (RefKind::Value, Weak::Type(Type::String))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            Expression::Identifier(String::from("foo")).to_weak(),
            (RefKind::Value, Weak::Unknown)
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
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn binary_subtract() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Subtract, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn binary_multiply() {
        assert_eq!(
            Expression::BinaryOperation(BinaryOperator::Multiply, Box::new(0), Box::new(1))
                .to_weak(),
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            Expression::DotAccess(Box::new(0), String::from("foo")).to_weak(),
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            Expression::FunctionCall(Box::new(0), vec![1]).to_weak(),
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            Expression::Style(vec![(String::from("foo"), 0)]).to_weak(),
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
