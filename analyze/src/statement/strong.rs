use crate::{
    context::StrongContext,
    infer::strong::{Strong, ToStrong},
};

impl<R> ToStrong<StatementNode<R, Strong>> for StatementNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> StatementNode<R, Strong> {
        StatementNode::new(
            self.node().value().map(&|x| x.to_strong(ctx)),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
