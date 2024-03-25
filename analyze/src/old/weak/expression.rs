use crate::ast;
use lang::{
    types::{RefKind, Type},
    NodeId,
};

impl super::ToWeak for ast::Expression<NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> super::Ref {
        match self {
            Self::Primitive(x) => (
                RefKind::Value,
                super::Type::Local(match x {
                    ast::Primitive::Nil => Type::Nil,
                    ast::Primitive::Boolean(..) => Type::Boolean,
                    ast::Primitive::Integer(..) => Type::Integer,
                    ast::Primitive::Float(..) => Type::Float,
                    ast::Primitive::String(..) => Type::String,
                }),
            ),

            Self::Identifier(x) => (
                RefKind::Value,
                super::Type::Infer(super::Inference::Resolve(x.clone())),
            ),

            Self::Group(x) => (RefKind::Value, super::Type::Inherit(**x)),

            Self::Closure(xs) => (RefKind::Value, {
                match xs.last() {
                    Some(id) => super::Type::Inherit(*id),
                    None => super::Type::Local(Type::Nil),
                }
            }),

            Self::UnaryOperation(op, id) => (
                RefKind::Value,
                match op {
                    ast::UnaryOperator::Not => super::Type::Local(Type::Boolean),

                    _ => super::Type::Inherit(**id),
                },
            ),

            Self::BinaryOperation(op, lhs, rhs) => (
                RefKind::Value,
                match op {
                    ast::BinaryOperator::Equal
                    | ast::BinaryOperator::NotEqual
                    | ast::BinaryOperator::And
                    | ast::BinaryOperator::Or
                    | ast::BinaryOperator::LessThan
                    | ast::BinaryOperator::LessThanOrEqual
                    | ast::BinaryOperator::GreaterThan
                    | ast::BinaryOperator::GreaterThanOrEqual => super::Type::Local(Type::Boolean),

                    ast::BinaryOperator::Divide | ast::BinaryOperator::Exponent => {
                        super::Type::Local(Type::Float)
                    }

                    ast::BinaryOperator::Add
                    | ast::BinaryOperator::Subtract
                    | ast::BinaryOperator::Multiply => {
                        super::Type::Infer(super::Inference::Arithmetic(**lhs, **rhs))
                    }
                },
            ),

            Self::PropertyAccess(x, property) => (
                RefKind::Value,
                super::Type::Infer(super::Inference::Property(**x, property.clone())),
            ),

            Self::FunctionCall(x, ..) => (
                RefKind::Value,
                super::Type::Infer(super::Inference::FunctionResult(**x)),
            ),

            Self::Style(..) => (RefKind::Value, super::Type::Local(Type::Style)),

            Self::Component(..) => (RefKind::Value, super::Type::Local(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{self, ToWeak},
    };
    use kore::str;
    use lang::{
        types::{RefKind, Type},
        NodeId,
    };

    #[test]
    fn primitive() {
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Nil).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Nil))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Boolean(true)).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Boolean))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Integer(123)).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Integer))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Float(123.45, 5)).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Float))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::String(str!("foo"))).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::String))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::Expression::Identifier(str!("foo")).to_weak(),
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Resolve(str!("foo")))
            )
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::Expression::Group(Box::new(NodeId(0))).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            ast::Expression::Closure(vec![NodeId(0), NodeId(1)]).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn closure_empty() {
        assert_eq!(
            ast::Expression::Closure(vec![]).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Nil))
        );
    }

    #[test]
    fn unary_not() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Not, Box::new(NodeId(0))).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Boolean))
        );
    }

    #[test]
    fn unary_absolute() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Absolute, Box::new(NodeId(0)))
                .to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn unary_negate() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Negate, Box::new(NodeId(0)))
                .to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Boolean))
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
            (RefKind::Value, weak::Type::Local(Type::Float))
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
            (RefKind::Value, weak::Type::Local(Type::Float))
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
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Arithmetic(NodeId(0), NodeId(1)))
            )
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
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Arithmetic(NodeId(0), NodeId(1)))
            )
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
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Arithmetic(NodeId(0), NodeId(1)))
            )
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::Expression::PropertyAccess(Box::new(NodeId(0)), str!("foo")).to_weak(),
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Property(NodeId(0), str!("foo")))
            )
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            ast::Expression::FunctionCall(Box::new(NodeId(0)), vec![NodeId(1)]).to_weak(),
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::FunctionResult(NodeId(0)))
            )
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            ast::Expression::Style(vec![(str!("foo"), NodeId(0))]).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Style))
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            ast::Expression::Component(Box::new(NodeId(0))).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Element))
        );
    }
}
