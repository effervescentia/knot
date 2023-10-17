use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{Strong, ToStrong},
    },
    ast::StatementNode,
    common::node::Node,
};

impl ToStrong<StatementNode<Strong>> for StatementNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> StatementNode<Strong> {
        StatementNode(Node(
            self.node().value().map(&|x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
