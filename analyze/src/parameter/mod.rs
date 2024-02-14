// mod fragment;
// mod identify;
// mod strong;
// mod weak;

// use super::{
//     context::{NodeContext, ScopeContext},
//     register::{Identify, Register},
// };
// use lang::ast::{AstNode, ParameterNode, ParameterNodeValue};

// impl<R> Register for ParameterNode<R, ()>
// where
//     R: Copy,
// {
//     type Node = ParameterNode<R, NodeContext>;
//     type Value<C> = ParameterNodeValue<R, C>;

//     fn register(&self, ctx: &ScopeContext) -> Self::Node {
//         let value = self.node().value().identify(ctx);
//         let id = ctx.add_fragment(&value);

//         ParameterNode::new(value, *self.node().range(), id)
//     }
// }

// #[cfg(test)]
// mod tests {
//     use crate::{
//         context::{FragmentMap, NodeContext},
//         fragment::Fragment,
//         register::Register,
//         test::fixture as f,
//     };
//     use kore::str;
//     use lang::ast::{Expression, Primitive, Statement};

//     #[test]
//     fn register() {
//         let file = &f::file_ctx();
//         let scope = &mut f::scope_ctx(file);

//         assert_eq!(
//             f::n::s(Statement::Variable(
//                 str!("foo"),
//                 f::n::x(Expression::Primitive(Primitive::Nil))
//             ))
//             .register(scope),
//             f::n::sc(
//                 Statement::Variable(
//                     str!("foo"),
//                     f::n::xc(
//                         Expression::Primitive(Primitive::Nil),
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
//                         Fragment::Statement(Statement::Variable(str!("foo"), 0))
//                     )
//                 )
//             ])
//         );
//     }
// }
