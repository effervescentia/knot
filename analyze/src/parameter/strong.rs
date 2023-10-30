use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::ParameterNode, Node};

impl<R> ToStrong<ParameterNode<R, Strong>> for ParameterNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ParameterNode<R, Strong> {
        ParameterNode(Node(
            self.node()
                .value()
                .map(&|x| x.to_strong(ctx), &|x| x.to_strong(ctx)),
            *self.node().range(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
