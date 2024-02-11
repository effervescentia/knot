// mod fragment;
// mod identify;
// mod strong;
mod weak;

// use super::{
//     context::NodeContext,
//     register::{Identify, Register},
//     ScopeContext,
// };
// use lang::ast::{AstNode, DeclarationNode, DeclarationNodeValue};

// impl<R> Register for DeclarationNode<R, ()>
// where
//     R: Copy,
// {
//     type Node = DeclarationNode<R, NodeContext>;
//     type Value<C> = DeclarationNodeValue<R, C>;

//     fn register(&self, ctx: &ScopeContext) -> DeclarationNode<R, NodeContext> {
//         let value = self.node().value().identify(&ctx.child());
//         let id = ctx.add_fragment(&value);

//         DeclarationNode::new(value, *self.node().range(), id)
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
//     use kore::{assert_eq, str};
//     use lang::ast::{
//         Expression, Import, ImportSource, Module, ModuleNode, Parameter, Primitive, TypeExpression,
//     };

//     #[test]
//     fn register_declaration() {
//         let file = &f::file_ctx();
//         let scope = &mut f::scope_ctx(file);

//         assert_eq!(
//             f::n::d(f::a::func_(
//                 "foo",
//                 vec![f::n::p(Parameter::new(
//                     str!("bar"),
//                     Some(f::n::tx(TypeExpression::Nil)),
//                     Some(f::n::x(Expression::Primitive(Primitive::Nil))),
//                 ),)],
//                 Some(f::n::tx(TypeExpression::Nil)),
//                 f::n::x(Expression::Primitive(Primitive::Nil)),
//             ))
//             .register(scope),
//             f::n::dc(
//                 f::a::func_(
//                     "foo",
//                     vec![f::n::pc(
//                         Parameter::new(
//                             str!("bar"),
//                             Some(f::n::txc(
//                                 TypeExpression::Nil,
//                                 NodeContext::new(0, vec![0, 1])
//                             )),
//                             Some(f::n::xc(
//                                 Expression::Primitive(Primitive::Nil),
//                                 NodeContext::new(1, vec![0, 1])
//                             )),
//                         ),
//                         NodeContext::new(2, vec![0, 1])
//                     )],
//                     Some(f::n::txc(
//                         TypeExpression::Nil,
//                         NodeContext::new(3, vec![0, 1])
//                     )),
//                     f::n::xc(
//                         Expression::Primitive(Primitive::Nil),
//                         NodeContext::new(4, vec![0, 1])
//                     )
//                 ),
//                 NodeContext::new(5, vec![0]),
//             )
//         );

//         assert_eq!(
//             scope.file.borrow().fragments,
//             FragmentMap::from_iter(vec![
//                 (
//                     0,
//                     (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
//                 ),
//                 (
//                     1,
//                     (
//                         vec![0, 1],
//                         Fragment::Expression(Expression::Primitive(Primitive::Nil))
//                     )
//                 ),
//                 (
//                     2,
//                     (
//                         vec![0, 1],
//                         Fragment::Parameter(Parameter::new(str!("bar"), Some(0), Some(1)))
//                     )
//                 ),
//                 (
//                     3,
//                     (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
//                 ),
//                 (
//                     4,
//                     (
//                         vec![0, 1],
//                         Fragment::Expression(Expression::Primitive(Primitive::Nil))
//                     )
//                 ),
//                 (
//                     5,
//                     (
//                         vec![0],
//                         Fragment::Declaration(f::a::func_("foo", vec![2], Some(3), 4))
//                     )
//                 )
//             ])
//         );
//     }

//     #[test]
//     fn register_module() {
//         let file = &f::file_ctx();
//         let scope = &mut f::scope_ctx(file);

//         assert_eq!(
//             f::n::d(f::a::module(
//                 "foo",
//                 f::n::mr(Module::new(
//                     vec![f::n::i(Import {
//                         source: ImportSource::Root,
//                         path: vec![str!("bar"), str!("fizz")],
//                         alias: Some(str!("Fizz")),
//                     })],
//                     vec![f::n::d(f::a::const_(
//                         "BUZZ",
//                         Some(f::n::tx(TypeExpression::Nil)),
//                         f::n::x(Expression::Primitive(Primitive::Nil)),
//                     ))],
//                 ))
//             ))
//             .register(scope),
//             f::n::dc(
//                 f::a::module(
//                     "foo",
//                     ModuleNode(
//                         Module::new(
//                             vec![f::n::ic(
//                                 Import {
//                                     source: ImportSource::Root,
//                                     path: vec![str!("bar"), str!("fizz")],
//                                     alias: Some(str!("Fizz")),
//                                 },
//                                 NodeContext::new(0, vec![0, 1])
//                             )],
//                             vec![f::n::dc(
//                                 f::a::const_(
//                                     "BUZZ",
//                                     Some(f::n::txc(
//                                         TypeExpression::Nil,
//                                         NodeContext::new(1, vec![0, 1, 3])
//                                     )),
//                                     f::n::xc(
//                                         Expression::Primitive(Primitive::Nil),
//                                         NodeContext::new(2, vec![0, 1, 3])
//                                     )
//                                 ),
//                                 NodeContext::new(3, vec![0, 1]),
//                             )],
//                         ),
//                         NodeContext::new(4, vec![0, 1]),
//                     )
//                 ),
//                 NodeContext::new(5, vec![0]),
//             )
//         );

//         assert_eq!(
//             scope.file.borrow().fragments,
//             FragmentMap::from_iter(vec![
//                 (
//                     0,
//                     (
//                         vec![0, 1],
//                         Fragment::Import(Import {
//                             source: ImportSource::Root,
//                             path: vec![str!("bar"), str!("fizz")],
//                             alias: Some(str!("Fizz")),
//                         })
//                     )
//                 ),
//                 (
//                     1,
//                     (vec![0, 1, 3], Fragment::TypeExpression(TypeExpression::Nil))
//                 ),
//                 (
//                     2,
//                     (
//                         vec![0, 1, 3],
//                         Fragment::Expression(Expression::Primitive(Primitive::Nil))
//                     )
//                 ),
//                 (
//                     3,
//                     (
//                         vec![0, 1],
//                         Fragment::Declaration(f::a::const_("BUZZ", Some(1), 2))
//                     )
//                 ),
//                 (
//                     4,
//                     (vec![0, 1], Fragment::Module(Module::new(vec![0], vec![3],)))
//                 ),
//                 (5, (vec![0], Fragment::Declaration(f::a::module("foo", 4))))
//             ])
//         );
//     }
// }
