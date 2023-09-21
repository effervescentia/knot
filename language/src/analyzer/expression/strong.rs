use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::expression::ExpressionNode,
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
        ExpressionNode(Node(
            self.node()
                .value()
                .map(&mut |x| x.to_strong(), &mut |x| x.to_strong(), &mut |x| {
                    x.to_strong()
                }),
            self.node().range().clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
