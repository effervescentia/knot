use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::StatementNode, Node};

impl<R> ToStrong<StatementNode<R, Strong>> for StatementNode<R, NodeContext>
where
    R: Clone,
{
    fn to_strong(&self, ctx: &StrongContext) -> StatementNode<R, Strong> {
        StatementNode(Node(
            self.node().value().map(&|x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
