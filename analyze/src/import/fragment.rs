use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::{AstNode, ImportNodeValue, ImportSourceNodeValue};

impl<R> ToFragment for ImportNodeValue<R, NodeContext>
where
    R: Copy,
{
    fn to_fragment(&self) -> Fragment {
        Fragment::Import(self.map(&|x| *x.node().id()))
    }
}

impl ToFragment for ImportSourceNodeValue {
    fn to_fragment(&self) -> Fragment {
        Fragment::ImportSource(self.clone())
    }
}
