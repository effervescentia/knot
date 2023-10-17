use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{Strong, ToStrong},
    },
    ast::KSXNode,
    common::node::Node,
};

impl ToStrong<KSXNode<Strong>> for KSXNode<NodeContext> {
    fn to_strong(&self, ctx: &StrongContext) -> KSXNode<Strong> {
        KSXNode(Node(
            self.node()
                .value()
                .map(&mut |x| x.to_strong(ctx), &mut |x| x.to_strong(ctx)),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}
