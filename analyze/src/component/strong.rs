use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::ast::{AstNode, KSXNode};

impl<R> ToStrong<KSXNode<R, Strong>> for KSXNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> KSXNode<R, Strong> {
        KSXNode::new(
            self.node()
                .value()
                .map(&mut |x| x.to_strong(ctx), &mut |x| x.to_strong(ctx)),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
