mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::{AstNode, ImportNode, ImportNodeValue, ImportSourceNode, ImportSourceNodeValue};

impl<R> Register for ImportNode<R, ()>
where
    R: Copy,
{
    type Node = ImportNode<R, NodeContext>;
    type Value<C> = ImportNodeValue<R, C>;

    fn register(&self, ctx: &ScopeContext) -> ImportNode<R, NodeContext> {
        let value = self.node().value().identify(&ctx.child());
        let id = ctx.add_fragment(&value);

        ImportNode::new(value, *self.node().range(), id)
    }
}

impl<R> Register for ImportSourceNode<R, ()>
where
    R: Copy,
{
    type Node = ImportSourceNode<R, NodeContext>;
    type Value<C> = ImportSourceNodeValue;

    fn register(&self, ctx: &ScopeContext) -> ImportSourceNode<R, NodeContext> {
        let value = self.node().value().identify(&ctx.child());
        let id = ctx.add_fragment(&value);

        ImportSourceNode::new(value, *self.node().range(), id)
    }
}
