mod fragment;
mod identify;
mod weak;
use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    parser::{
        expression::{self, ExpressionNode},
        node::Node,
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for ExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = ExpressionNode<T, NodeContext>;
    type Value<C> = expression::NodeValue<T, C>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node {
        let node = self.0;
        let value = node.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        ExpressionNode(Node(value, node.1, id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        parser::{
            expression::{
                binary_operation::BinaryOperator, primitive::Primitive, Expression, UnaryOperator,
            },
            ksx::KSX,
            statement::Statement,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;

    #[test]
    fn primitive() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Nil)).register(scope),
            f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![(
                0,
                (
                    vec![0],
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                )
            )])
        );
    }

    #[test]
    fn identifier() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::Identifier(String::from("foo"))).register(scope),
            f::n::xc(
                Expression::Identifier(String::from("foo")),
                NodeContext::new(0, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![(
                0,
                (
                    vec![0],
                    Fragment::Expression(Expression::Identifier(String::from("foo")))
                )
            )])
        );
    }

    #[test]
    fn group() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::Group(Box::new(f::n::x(Expression::Primitive(
                Primitive::Nil
            )))))
            .register(scope),
            f::n::xc(
                Expression::Group(Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                ))),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Group(Box::new(0)))
                    )
                ),
            ])
        );
    }

    #[test]
    fn closure() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::Closure(vec![
                f::n::s(Statement::Variable(
                    String::from("foo"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                )),
                f::n::s(Statement::Effect(f::n::x(Expression::Primitive(
                    Primitive::Nil
                )))),
            ]))
            .register(scope),
            f::n::xc(
                Expression::Closure(vec![
                    f::n::sc(
                        Statement::Variable(
                            String::from("foo"),
                            f::n::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(0, vec![0, 1])
                            ),
                        ),
                        NodeContext::new(1, vec![0, 1])
                    ),
                    f::n::sc(
                        Statement::Effect(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(2, vec![0, 1])
                        )),
                        NodeContext::new(3, vec![0, 1])
                    )
                ]),
                NodeContext::new(4, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Statement(Statement::Variable(String::from("foo"), 0))
                    )
                ),
                (
                    2,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (3, (vec![0, 1], Fragment::Statement(Statement::Effect(2)))),
                (
                    4,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Closure(vec![1, 3]))
                    )
                ),
            ])
        );
    }

    #[test]
    fn unary_operation() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
            ))
            .register(scope),
            f::n::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::UnaryOperation(
                            UnaryOperator::Not,
                            Box::new(0)
                        ))
                    )
                ),
            ])
        );
    }

    #[test]
    fn binary_operation() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
            ))
            .register(scope),
            f::n::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    Box::new(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(1, vec![0])
                    )),
                ),
                NodeContext::new(2, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Expression(Expression::BinaryOperation(
                            BinaryOperator::Equal,
                            Box::new(0),
                            Box::new(1),
                        ))
                    )
                ),
            ])
        );
    }

    #[test]
    fn dot_access() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::DotAccess(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                String::from("foo"),
            ))
            .register(scope),
            f::n::xc(
                Expression::DotAccess(
                    Box::new(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    String::from("foo"),
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::DotAccess(
                            Box::new(0),
                            String::from("foo")
                        ))
                    )
                ),
            ])
        );
    }

    #[test]
    fn function_call() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::FunctionCall(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                vec![
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ],
            ))
            .register(scope),
            f::n::xc(
                Expression::FunctionCall(
                    Box::new(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    vec![
                        f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        ),
                        f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(2, vec![0])
                        ),
                    ],
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Expression(Expression::FunctionCall(Box::new(0), vec![1, 2]))
                    )
                ),
            ])
        );
    }

    #[test]
    fn style() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::Style(vec![
                (
                    String::from("foo"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ),
                (
                    String::from("bar"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ),
            ]))
            .register(scope),
            f::n::xc(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(0, vec![0])
                        ),
                    ),
                    (
                        String::from("bar"),
                        f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        ),
                    ),
                ]),
                NodeContext::new(2, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Style(vec![
                            (String::from("foo"), 0),
                            (String::from("bar"), 1),
                        ]))
                    )
                ),
            ])
        );
    }

    #[test]
    fn ksx() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::x(Expression::KSX(Box::new(f::n::kx(KSX::Text(
                String::from("foo")
            )))))
            .register(scope),
            f::n::xc(
                Expression::KSX(Box::new(f::n::kxc(
                    KSX::Text(String::from("foo")),
                    NodeContext::new(0, vec![0])
                ))),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (0, (vec![0], Fragment::KSX(KSX::Text(String::from("foo"))))),
                (
                    1,
                    (vec![0], Fragment::Expression(Expression::KSX(Box::new(0))))
                ),
            ])
        );
    }
}
