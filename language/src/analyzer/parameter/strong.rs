use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{Strong, ToStrong},
    },
    ast::parameter::ParameterNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, ParameterNode<T, Strong>> for ParameterNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a StrongContext<'a>) -> ParameterNode<T, Strong> {
        ParameterNode(Node(
            self.node()
                .value()
                .map(&|x| x.to_strong(ctx), &|x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.get_strong_or_fail(self.node().id()).clone(),
        ))
    }
}
