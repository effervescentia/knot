use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::TypeExpressionNode, Node};

impl<R> ToStrong<TypeExpressionNode<R, Strong>> for TypeExpressionNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> TypeExpressionNode<R, Strong> {
        TypeExpressionNode(Node(
            self.node().value().map(&|x| x.to_strong(ctx)),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
