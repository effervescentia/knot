use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::statement::StatementNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToStrong<StatementNode<T, Strong>> for StatementNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self) -> StatementNode<T, Strong> {
        StatementNode(Node(
            self.node().value().map(&|x| x.to_strong()),
            self.node().range().clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
