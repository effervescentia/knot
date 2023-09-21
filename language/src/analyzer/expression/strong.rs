use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::expression::{Expression, ExpressionNode},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToStrong<ExpressionNode<T, Strong>> for ExpressionNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self) -> ExpressionNode<T, Strong> {
        let node = self.node();
        ExpressionNode(Node(
            match node.0 {
                Expression::Primitive(x) => Expression::Primitive(x),

                Expression::Identifier(x) => Expression::Identifier(x),

                Expression::Group(x) => Expression::Group(Box::new(x.to_strong())),

                Expression::Closure(xs) => {
                    Expression::Closure(xs.into_iter().map(|x| x.to_strong()).collect::<Vec<_>>())
                }

                Expression::UnaryOperation(op, x) => {
                    Expression::UnaryOperation(op, Box::new(x.to_strong()))
                }

                Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                    op,
                    Box::new(lhs.to_strong()),
                    Box::new(rhs.to_strong()),
                ),

                Expression::DotAccess(lhs, rhs) => {
                    Expression::DotAccess(Box::new(lhs.to_strong()), rhs)
                }

                Expression::FunctionCall(x, xs) => Expression::FunctionCall(
                    Box::new(x.to_strong()),
                    xs.into_iter().map(|x| x.to_strong()).collect::<Vec<_>>(),
                ),

                Expression::Style(xs) => Expression::Style(
                    xs.into_iter()
                        .map(|(key, value)| (key, value.to_strong()))
                        .collect::<Vec<_>>(),
                ),

                Expression::KSX(x) => Expression::KSX(Box::new(x.to_strong())),
            },
            node.1.clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
