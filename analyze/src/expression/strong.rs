use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::ExpressionNode, Node};

impl<R> ToStrong<ExpressionNode<R, Strong>> for ExpressionNode<R, NodeContext>
where
    R: Clone,
{
    fn to_strong(&self, ctx: &StrongContext) -> ExpressionNode<R, Strong> {
        ExpressionNode(Node(
            self.node().value().map(
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
            ),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
