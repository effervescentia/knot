use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::TypeExpressionNode, Node};

impl ToStrong<TypeExpressionNode<Strong>> for TypeExpressionNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> TypeExpressionNode<Strong> {
        TypeExpressionNode(Node(
            self.node().value().map(&|x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
