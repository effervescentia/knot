use super::ScopeContext;
use crate::{
    context::NodeContext,
    register::{Identify, Register},
};
use lang::ast::{ImportNodeValue, ImportSourceNodeValue};

impl<R> Identify<ImportNodeValue<R, NodeContext>> for ImportNodeValue<R, ()>
where
    R: Copy,
{
    fn identify(&self, ctx: &ScopeContext) -> ImportNodeValue<R, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

impl Identify<Self> for ImportSourceNodeValue {
    fn identify(&self, _ctx: &ScopeContext) -> Self {
        self.clone()
    }
}
