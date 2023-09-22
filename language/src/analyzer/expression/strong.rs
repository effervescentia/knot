use crate::{
    analyzer::{
        context::{AnalyzeContext, NodeContext},
        infer::strong::ToStrong,
        Strong,
    },
    ast::expression::ExpressionNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, ExpressionNode<T, Strong>> for ExpressionNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a AnalyzeContext<'a>) -> ExpressionNode<T, Strong> {
        ExpressionNode(Node(
            self.node().value().map(
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
            ),
            self.node().range().clone(),
            ctx.get_strong_or_fail(self.node().id()).clone(),
        ))
    }
}
