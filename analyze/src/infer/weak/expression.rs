use super::{
    data::{Inference, Type, Weak},
    to_weak::ToWeak,
};
use crate::AmbientScope;
use lang::{
    ast,
    types::{self, Kind},
    NodeId,
};

impl ToWeak for ast::Expression<NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Primitive(x) => (
                Kind::Value,
                Type::Value(match x {
                    ast::Primitive::Nil => types::Type::Nil,
                    ast::Primitive::Boolean(..) => types::Type::Boolean,
                    ast::Primitive::Integer(..) => types::Type::Integer,
                    ast::Primitive::Float(..) => types::Type::Float,
                    ast::Primitive::String(..) => types::Type::String,
                }),
            ),

            Self::Identifier(x) => (
                Kind::Value,
                Type::Infer(Inference::Reference(x.clone(), None)),
            ),

            Self::Group(x) => (Kind::Value, Type::Inherit(**x)),

            Self::Closure(xs) => (Kind::Value, {
                match xs.last() {
                    Some(id) => Type::Inherit(*id),
                    None => Type::Value(types::Type::Nil),
                }
            }),

            Self::UnaryOperation(op, id) => (
                Kind::Value,
                match op {
                    ast::UnaryOperator::Not => Type::Value(types::Type::Boolean),

                    ast::UnaryOperator::Absolute | ast::UnaryOperator::Negate => {
                        Type::Inherit(**id)
                    }
                },
            ),

            Self::BinaryOperation(op, lhs, rhs) => (
                Kind::Value,
                match op {
                    ast::BinaryOperator::Equal
                    | ast::BinaryOperator::NotEqual
                    | ast::BinaryOperator::And
                    | ast::BinaryOperator::Or
                    | ast::BinaryOperator::LessThan
                    | ast::BinaryOperator::LessThanOrEqual
                    | ast::BinaryOperator::GreaterThan
                    | ast::BinaryOperator::GreaterThanOrEqual => Type::Value(types::Type::Boolean),

                    ast::BinaryOperator::Divide | ast::BinaryOperator::Exponent => {
                        Type::Value(types::Type::Float)
                    }

                    ast::BinaryOperator::Add
                    | ast::BinaryOperator::Subtract
                    | ast::BinaryOperator::Multiply => {
                        Type::Infer(Inference::Arithmetic(*op, **lhs, **rhs))
                    }
                },
            ),

            Self::PropertyAccess(x, property) => (
                Kind::Value,
                Type::Infer(Inference::Property(**x, property.clone())),
            ),

            Self::FunctionCall(x, ..) => (Kind::Value, Type::Infer(Inference::Product(**x))),

            Self::Style(..) => (Kind::Value, Type::Value(types::Type::Style)),

            Self::Component(_) => (Kind::Value, Type::Value(types::Type::Element)),
        }
    }
}

impl ToWeak for ast::Statement<NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Expression(id) => (Kind::Value, Type::Inherit(*id)),

            Self::Variable(..) => (Kind::Value, Type::Value(types::Type::Nil)),
        }
    }
}

impl ToWeak for ast::Attribute<NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Punned(name) => (
                Kind::Value,
                Type::Infer(Inference::Reference(name.clone(), None)),
            ),

            Self::Explicit(_, id) => (Kind::Value, Type::Inherit(*id)),
        }
    }
}

impl ToWeak for ast::Component<NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Text(..) => (Kind::Value, Type::Value(types::Type::String)),

            Self::Expression(id) => (Kind::Value, Type::Inherit(*id)),

            Self::Fragment(..) => (Kind::Value, Type::Value(types::Type::Element)),

            Self::ClosedElement(tag, _) | Self::OpenElement { start_tag: tag, .. } => (
                Kind::Value,
                Type::Infer(Inference::Reference(
                    tag.clone(),
                    Some(AmbientScope::Element),
                )),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{Inference, ToWeak, Type};
    use crate::AmbientScope;
    use kore::str;
    use lang::{
        ast,
        types::{self, Kind},
        NodeId,
    };

    #[test]
    fn expression_primitive() {
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Nil).to_weak(),
            (Kind::Value, Type::Value(types::Type::Nil))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Boolean(true)).to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Integer(123)).to_weak(),
            (Kind::Value, Type::Value(types::Type::Integer))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::Float(45.67, 2)).to_weak(),
            (Kind::Value, Type::Value(types::Type::Float))
        );
        assert_eq!(
            ast::Expression::Primitive(ast::Primitive::String(str!("foo"))).to_weak(),
            (Kind::Value, Type::Value(types::Type::String))
        );
    }

    #[test]
    fn expression_identifier() {
        assert_eq!(
            ast::Expression::Identifier(str!("foo")).to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Reference(str!("foo"), None))
            )
        );
    }

    #[test]
    fn expression_group() {
        assert_eq!(
            ast::Expression::Group(Box::new(NodeId(1))).to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn expression_closure_empty() {
        assert_eq!(
            ast::Expression::Closure(vec![]).to_weak(),
            (Kind::Value, Type::Value(types::Type::Nil))
        );
    }

    #[test]
    fn expression_closure() {
        assert_eq!(
            ast::Expression::Closure(vec![NodeId(1), NodeId(2)]).to_weak(),
            (Kind::Value, Type::Inherit(NodeId(2)))
        );
    }

    #[test]
    fn expression_unary_not() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Not, Box::new(NodeId(1))).to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
    }

    #[test]
    fn expression_unary() {
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Absolute, Box::new(NodeId(1)))
                .to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
        assert_eq!(
            ast::Expression::UnaryOperation(ast::UnaryOperator::Negate, Box::new(NodeId(1)))
                .to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn expression_binary_simple_arithmetic() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Add,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Arithmetic(
                    ast::BinaryOperator::Add,
                    NodeId(1),
                    NodeId(2)
                ))
            )
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Subtract,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Arithmetic(
                    ast::BinaryOperator::Subtract,
                    NodeId(1),
                    NodeId(2)
                ))
            )
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Multiply,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Arithmetic(
                    ast::BinaryOperator::Multiply,
                    NodeId(1),
                    NodeId(2)
                ))
            )
        );
    }

    #[test]
    fn expression_binary_complex_arithmetic() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Divide,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Float))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Exponent,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Float))
        );
    }

    #[test]
    fn expression_binary() {
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Equal,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::NotEqual,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::And,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::Or,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::LessThan,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::LessThanOrEqual,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::GreaterThan,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::Expression::BinaryOperation(
                ast::BinaryOperator::GreaterThanOrEqual,
                Box::new(NodeId(1)),
                Box::new(NodeId(2))
            )
            .to_weak(),
            (Kind::Value, Type::Value(types::Type::Boolean))
        );
    }

    #[test]
    fn expression_property() {
        assert_eq!(
            ast::Expression::PropertyAccess(Box::new(NodeId(1)), str!("foo")).to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Property(NodeId(1), str!("foo")))
            )
        );
    }

    #[test]
    fn expression_function_result() {
        assert_eq!(
            ast::Expression::FunctionCall(Box::new(NodeId(1)), vec![NodeId(2), NodeId(3)])
                .to_weak(),
            (Kind::Value, Type::Infer(Inference::Product(NodeId(1))))
        );
    }

    #[test]
    fn expression_style() {
        assert_eq!(
            ast::Expression::Style(vec![]).to_weak(),
            (Kind::Value, Type::Value(types::Type::Style))
        );
    }

    #[test]
    fn expression_component() {
        assert_eq!(
            ast::Expression::Component(Box::new(NodeId(1))).to_weak(),
            (Kind::Value, Type::Value(types::Type::Element))
        );
    }

    #[test]
    fn statement_expression() {
        assert_eq!(
            ast::Statement::Expression(NodeId(1)).to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn statement_variable() {
        assert_eq!(
            ast::Statement::Variable(str!("foo"), NodeId(1)).to_weak(),
            (Kind::Value, Type::Value(types::Type::Nil))
        );
    }

    #[test]
    fn component_text() {
        assert_eq!(
            ast::Component::Text(str!("foo")).to_weak(),
            (Kind::Value, Type::Value(types::Type::String))
        );
    }

    #[test]
    fn component_expression() {
        assert_eq!(
            ast::Component::Expression(NodeId(1)).to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn component_elements() {
        assert_eq!(
            ast::Component::Fragment(vec![]).to_weak(),
            (Kind::Value, Type::Value(types::Type::Element))
        );
        assert_eq!(
            ast::Component::ClosedElement(str!("div"), vec![]).to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Reference(
                    str!("div"),
                    Some(AmbientScope::Element)
                ))
            )
        );
        assert_eq!(
            ast::Component::open_element(str!("main"), vec![], vec![], str!("main")).to_weak(),
            (
                Kind::Value,
                Type::Infer(Inference::Reference(
                    str!("main"),
                    Some(AmbientScope::Element)
                ))
            )
        );
    }
}
