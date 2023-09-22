use crate::{
    analyzer::{
        context::{AnalyzeContext, NodeContext},
        infer::strong::ToStrong,
        Strong,
    },
    ast::declaration::DeclarationNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, DeclarationNode<T, Strong>> for DeclarationNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a AnalyzeContext<'a>) -> DeclarationNode<T, Strong> {
        DeclarationNode(Node(
            self.node().value().map(
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
            ),
            self.node().range().clone(),
            ctx.get_strong_or_fail(self.node().id()).clone(),
        ))
    }
}
