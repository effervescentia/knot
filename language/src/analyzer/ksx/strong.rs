use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{Strong, ToStrong},
    },
    ast::ksx::KSXNode,
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, KSXNode<T, Strong>> for KSXNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a StrongContext<'a>) -> KSXNode<T, Strong> {
        KSXNode(Node(
            self.node()
                .value()
                .map(&mut |x| x.to_strong(ctx), &mut |x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.get_strong_or_fail(self.node().id()).clone(),
        ))
    }
}
