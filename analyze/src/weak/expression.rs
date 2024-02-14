use crate::{
    ast::{self, walk},
    weak::{ToWeak, Weak, WeakRef},
};
use lang::types;

impl ToWeak for ast::Expression<walk::NodeId, walk::NodeId, walk::NodeId> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Primitive(x) => (
                types::RefKind::Value,
                Weak::Type(match x {
                    ast::Primitive::Nil => types::Type::Nil,
                    ast::Primitive::Boolean(..) => types::Type::Boolean,
                    ast::Primitive::Integer(..) => types::Type::Integer,
                    ast::Primitive::Float(..) => types::Type::Float,
                    ast::Primitive::String(..) => types::Type::String,
                }),
            ),

            Self::Identifier(..) => (types::RefKind::Value, Weak::Infer),

            Self::Group(id) => (types::RefKind::Value, Weak::Inherit(**id)),

            Self::Closure(xs) => (types::RefKind::Value, {
                match xs.last() {
                    Some(id) => Weak::Inherit(*id),
                    None => Weak::Type(types::Type::Nil),
                }
            }),

            Self::UnaryOperation(op, id) => (
                types::RefKind::Value,
                match op {
                    ast::UnaryOperator::Not => Weak::Type(types::Type::Boolean),

                    _ => Weak::Inherit(**id),
                },
            ),

            Self::BinaryOperation(op, ..) => (
                types::RefKind::Value,
                match op {
                    ast::BinaryOperator::Equal
                    | ast::BinaryOperator::NotEqual
                    | ast::BinaryOperator::And
                    | ast::BinaryOperator::Or
                    | ast::BinaryOperator::LessThan
                    | ast::BinaryOperator::LessThanOrEqual
                    | ast::BinaryOperator::GreaterThan
                    | ast::BinaryOperator::GreaterThanOrEqual => Weak::Type(types::Type::Boolean),

                    ast::BinaryOperator::Divide | ast::BinaryOperator::Exponent => {
                        Weak::Type(types::Type::Float)
                    }

                    ast::BinaryOperator::Add
                    | ast::BinaryOperator::Subtract
                    | ast::BinaryOperator::Multiply => Weak::Infer,
                },
            ),

            Self::PropertyAccess(..) => (types::RefKind::Value, Weak::Infer),

            Self::FunctionCall(..) => (types::RefKind::Value, Weak::Infer),

            Self::Style(..) => (types::RefKind::Value, Weak::Type(types::Type::Style)),

            Self::Component(..) => (types::RefKind::Value, Weak::Type(types::Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{self, walk::NodeId},
        weak::{ToWeak, Weak},
    };
    use kore::str;
    use lang::types;

    #[test]
    fn primitive() {
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Nil).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Nil))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Boolean(true)).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Integer(123)).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Integer))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Float(123.45, 5)).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Float))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::String(str!("foo"))).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::String))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::Expression::Identifier(str!("foo")).to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::Expression::Group(Box::new(NodeId(0))).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            ast::Expression::Closure(vec![NodeId(0), NodeId(1)]).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn closure_empty() {
        assert_eq!(
            ast::Expression::Closure(vec![]).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Nil))
        );
    }

    #[test]
    fn unary_not() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Not, Box::new(NodeId(0))).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn unary_absolute() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Absolute, Box::new(NodeId(0)))
                .to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn unary_negate() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Negate, Box::new(NodeId(0)))
                .to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn binary_equal() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Equal,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_not_equal() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::NotEqual,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_and() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::And,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_or() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Or,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_less_than() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::LessThan,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_less_than_or_equal() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::LessThanOrEqual,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_greater_than() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::GreaterThan,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_greater_than_or_equal() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::GreaterThanOrEqual,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Boolean))
        );
    }

    #[test]
    fn binary_divide() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Divide,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Float))
        );
    }

    #[test]
    fn binary_exponent() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Exponent,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Float))
        );
    }

    #[test]
    fn binary_add() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Add,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn binary_subtract() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Subtract,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn binary_multiply() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Multiply,
                Box::new(NodeId(0)),
                Box::new(NodeId(1))
            )
            .to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::Expression::PropertyAccess(Box::new(NodeId(0)), str!("foo")).to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            ast::Expression::FunctionCall(Box::new(NodeId(0)), vec![NodeId(1)]).to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            ast::Expression::Style(vec![(str!("foo"), NodeId(0))]).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Style))
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            ast::Expression::Component(Box::new(NodeId(0))).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Element))
        );
    }
}
