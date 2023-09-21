use crate::{
    analyzer::{
        context::{AnalyzeContext, NodeContext},
        infer::strong::ToStrong,
        Strong, Type,
    },
    ast::type_expression::TypeExpressionNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, TypeExpressionNode<T, Strong>> for TypeExpressionNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a AnalyzeContext<'a>) -> TypeExpressionNode<T, Strong> {
        TypeExpressionNode(Node(
            self.node().value().map(&|x| x.to_strong(ctx)),
            self.node().range().clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
