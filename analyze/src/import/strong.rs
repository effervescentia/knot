use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::ast::{AstNode, ImportNode, ImportSourceNode, TypedNode};

impl<R> ToStrong<ImportNode<R, Strong>> for ImportNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ImportNode<R, Strong> {
        ImportNode::new(
            self.node().value().map(&|x| x.to_strong(ctx)),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}

impl<R> ToStrong<ImportSourceNode<R, Strong>> for ImportSourceNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ImportSourceNode<R, Strong> {
        TypedNode::new(
            self.node().value().clone(),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
