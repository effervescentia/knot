use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::DeclarationNode, Node};

impl<R> ToStrong<DeclarationNode<R, Strong>> for DeclarationNode<R, NodeContext>
where
    R: Clone,
{
    fn to_strong(&self, ctx: &StrongContext) -> DeclarationNode<R, Strong> {
        DeclarationNode(Node(
            self.node().value().map(
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
                &|x| x.to_strong(ctx),
            ),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
