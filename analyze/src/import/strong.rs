use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::ImportNode, Node};

impl<R> ToStrong<ImportNode<R, Strong>> for ImportNode<R, NodeContext>
where
    R: Clone,
{
    fn to_strong(&self, ctx: &StrongContext) -> ImportNode<R, Strong> {
        ImportNode(Node(
            self.node().value().clone(),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
