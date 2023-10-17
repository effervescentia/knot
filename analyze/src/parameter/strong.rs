use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::ParameterNode, Node};

impl ToStrong<ParameterNode<Strong>> for ParameterNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> ParameterNode<Strong> {
        ParameterNode(Node(
            self.node()
                .value()
                .map(&|x| x.to_strong(ctx), &|x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
