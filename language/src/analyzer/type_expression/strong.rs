use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::type_expression::TypeExpressionNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToStrong<TypeExpressionNode<T, Strong>> for TypeExpressionNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self) -> TypeExpressionNode<T, Strong> {
        TypeExpressionNode(Node(
            self.node().value().map(&|x| x.to_strong()),
            self.node().range().clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
