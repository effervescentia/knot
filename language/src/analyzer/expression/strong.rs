use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{Strong, ToStrong},
    },
    ast::ExpressionNode,
    common::node::Node,
};

impl ToStrong<ExpressionNode<Strong>> for ExpressionNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> ExpressionNode<Strong> {
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
