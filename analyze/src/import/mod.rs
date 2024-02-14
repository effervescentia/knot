// mod fragment;
// mod identify;
// mod strong;
// mod weak;

// use super::{
//     context::{NodeContext, ScopeContext},
//     register::{Identify, Register},
// };
// use lang::ast::{AstNode, ImportNode, ImportNodeValue};

// impl<R> Register for ImportNode<R, ()>
// where
//     R: Copy,
// {
//     type Node = ImportNode<R, NodeContext>;
//     type Value<C> = ImportNodeValue;

//     fn register(&self, ctx: &ScopeContext) -> ImportNode<R, NodeContext> {
//         let value = self.node().value().identify(&ctx.child());
//         let id = ctx.add_fragment(&value);

//         ImportNode::new(value, *self.node().range(), id)
//     }
// }
