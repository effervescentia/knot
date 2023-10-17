use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::DeclarationNode, Node};

impl ToStrong<DeclarationNode<Strong>> for DeclarationNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> DeclarationNode<Strong> {
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
