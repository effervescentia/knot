use crate::{
    context::{NodeContext, StrongContext},
    infer::strong::{Strong, ToStrong},
};
use lang::{ast::KSXNode, Node};

impl<R> ToStrong<KSXNode<R, Strong>> for KSXNode<R, NodeContext>
where
    R: Clone,
{
    fn to_strong(&self, ctx: &StrongContext) -> KSXNode<R, Strong> {
        KSXNode(Node(
            self.node()
                .value()
                .map(&mut |x| x.to_strong(ctx), &mut |x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
