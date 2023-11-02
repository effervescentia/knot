use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::ast::{AstNode, ImportNode};

impl<R> ToStrong<ImportNode<R, Strong>> for ImportNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ImportNode<R, Strong> {
        ImportNode::new(
            self.node().value().clone(),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
