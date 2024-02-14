// mod fragment;
// mod identify;
// pub mod strong;
// mod weak;

// use crate::context::{NodeContext, ScopeContext};
use lang::ast;

// impl<R> Register for ExpressionNode<R, ()>
// where
//     R: Copy,
// {
//     type Node = ExpressionNode<R, NodeContext>;
//     type Value<C> = ExpressionNodeValue<R, C>;

//     fn register(&self, ctx: &ScopeContext) -> Self::Node {
//         let value = self.node().value().identify(ctx);
//         let id = ctx.add_fragment(&value);

//         ExpressionNode::new(value, *self.node().range(), id)
//     }
// }

#[cfg(test)]
mod tests {
    use crate::test::fixture as f;
    use kore::str;
    // use lang::ast::{Expression, Primitive, Statement, KSX};

    // #[test]
    // fn register_closure() {
    //     let file = &f::file_ctx();
    //     let scope = &mut f::scope_ctx(file);

    //     assert_eq!(
    //         f::n::x(Expression::Closure(vec![
    //             f::n::s(Statement::Variable(
    //                 str!("foo"),
    //                 f::n::x(Expression::Primitive(Primitive::Nil)),
    //             )),
    //             f::n::s(Statement::Expression(f::n::x(Expression::Primitive(
    //                 Primitive::Nil
    //             )))),
    //         ]))
    //         .register(scope),
    //         f::n::xc(
    //             Expression::Closure(vec![
    //                 f::n::sc(
    //                     Statement::Variable(
    //                         str!("foo"),
    //                         f::n::xc(
    //                             Expression::Primitive(Primitive::Nil),
    //                             NodeContext::new(0, vec![0, 1])
    //                         ),
    //                     ),
    //                     NodeContext::new(1, vec![0, 1])
    //                 ),
    //                 f::n::sc(
    //                     Statement::Expression(f::n::xc(
    //                         Expression::Primitive(Primitive::Nil),
    //                         NodeContext::new(2, vec![0, 1])
    //                     )),
    //                     NodeContext::new(3, vec![0, 1])
    //                 )
    //             ]),
    //             NodeContext::new(4, vec![0]),
    //         )
    //     );

    //     assert_eq!(
    //         scope.file.borrow().fragments,
    //         FragmentMap::from_iter(vec![
    //             (
    //                 0,
    //                 (
    //                     vec![0, 1],
    //                     Fragment::Expression(Expression::Primitive(Primitive::Nil))
    //                 )
    //             ),
    //             (
    //                 1,
    //                 (
    //                     vec![0, 1],
    //                     Fragment::Statement(Statement::Variable(str!("foo"), 0))
    //                 )
    //             ),
    //             (
    //                 2,
    //                 (
    //                     vec![0, 1],
    //                     Fragment::Expression(Expression::Primitive(Primitive::Nil))
    //                 )
    //             ),
    //             (
    //                 3,
    //                 (vec![0, 1], Fragment::Statement(Statement::Expression(2)))
    //             ),
    //             (
    //                 4,
    //                 (
    //                     vec![0],
    //                     Fragment::Expression(Expression::Closure(vec![1, 3]))
    //                 )
    //             ),
    //         ])
    //     );
    // }

    // #[test]
    // fn register_ksx() {
    //     let file = &f::file_ctx();
    //     let scope = &mut f::scope_ctx(file);

    //     assert_eq!(
    //         f::n::x(Expression::Component(Box::new(f::n::kx(KSX::Text(str!(
    //             "foo"
    //         ))))))
    //         .register(scope),
    //         f::n::xc(
    //             Expression::Component(Box::new(f::n::kxc(
    //                 KSX::Text(str!("foo")),
    //                 NodeContext::new(0, vec![0])
    //             ))),
    //             NodeContext::new(1, vec![0]),
    //         )
    //     );

    //     assert_eq!(
    //         scope.file.borrow().fragments,
    //         FragmentMap::from_iter(vec![
    //             (0, (vec![0], Fragment::KSX(KSX::Text(str!("foo"))))),
    //             (
    //                 1,
    //                 (
    //                     vec![0],
    //                     Fragment::Expression(Expression::Component(Box::new(0)))
    //                 )
    //             ),
    //         ])
    //     );
    // }
}
