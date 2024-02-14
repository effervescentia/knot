// mod fragment;
// mod identity;
// mod strong;
// mod weak;

// use super::{
//     context::NodeContext,
//     infer::weak::Weak,
//     register::{Identify, Register},
//     RefKind, ScopeContext, Type,
// };
// use lang::ast::{AstNode, StatementNode, StatementNodeValue};

// impl<R> Register for StatementNode<R, ()>
// where
//     R: Copy,
// {
//     type Node = StatementNode<R, NodeContext>;
//     type Value<C> = StatementNodeValue<R, C>;

//     fn register(&self, ctx: &ScopeContext) -> Self::Node {
//         let node = self.node();
//         let value = node.value().identify(ctx);
//         let id = ctx.add_fragment(&value);

//         StatementNode::new(value, *node.range(), id)
//     }
// }

// #[cfg(test)]
// mod tests {
//     use crate::test::fixture as f;
//     use kore::str;
//     use lang::ast::{
//         self,
//         explode::{Fragment, FragmentMap},
//     };

//     #[test]
//     fn register() {
//         let file = &f::file_ctx();
//         let scope = &mut f::scope_ctx(file);

//         assert_eq!(
//             f::n::s(ast::Statement::Variable(
//                 str!("foo"),
//                 f::n::x(ast::Expression::Primitive(ast::Primitive::Nil))
//             ))
//             .register(scope),
//             f::n::sc(
//                 ast::Statement::Variable(
//                     str!("foo"),
//                     f::n::xc(
//                         ast::Expression::Primitive(ast::Primitive::Nil),
//                         NodeContext::new(0, vec![0])
//                     )
//                 ),
//                 NodeContext::new(1, vec![0])
//             )
//         );

//         assert_eq!(
//             scope.file.borrow().fragments,
//             FragmentMap::from_iter(vec![
//                 (
//                     0,
//                     (
//                         vec![0],
//                         Fragment::Expression(Expression::Primitive(Primitive::Nil))
//                     )
//                 ),
//                 (
//                     1,
//                     (
//                         vec![0],
//                         Fragment::Statement(ast::Statement::Variable(str!("foo"), 0))
//                     )
//                 )
//             ])
//         );
//     }
// }
