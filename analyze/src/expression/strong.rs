use crate::{
    context::StrongContext,
    infer::strong::{Strong, ToStrong},
};
use lang::ast;

impl<R> ToStrong<ExpressionNode<R, Strong>> for ExpressionNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ExpressionNode<R, Strong> {
        ExpressionNode::new(
            self.node().value().map(
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
            ),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
