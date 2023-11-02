use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::ast::{AstNode, DeclarationNode};

impl<R> ToStrong<DeclarationNode<R, Strong>> for DeclarationNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> DeclarationNode<R, Strong> {
        DeclarationNode::new(
            self.node().value().map(
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
            ),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        )
    }
}
