use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
use crate::parser::{
    expression::{
        ksx::KSXNode,
        statement::{Statement, StatementNode},
        Expression, ExpressionNode,
    },
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Analyze<ExpressionNode<T, NodeContext>, Expression<usize, usize, usize>>
    for ExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = Expression<ExpressionNode<T, C>, StatementNode<T, C>, KSXNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> ExpressionNode<T, NodeContext> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::Expression(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        ExpressionNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
        match value {
            Expression::Primitive(x) => Expression::Primitive(x),

            Expression::Identifier(x) => Expression::Identifier(x),

            Expression::Group(x) => Expression::Group(Box::new((*x).register(ctx))),

            Expression::Closure(xs) => {
                let child_ctx = &mut ctx.child();

                Expression::Closure(
                    xs.into_iter()
                        .map(|x| x.register(child_ctx))
                        .collect::<Vec<_>>(),
                )
            }

            Expression::UnaryOperation(op, lhs) => {
                Expression::UnaryOperation(op, Box::new(lhs.register(ctx)))
            }

            Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                op,
                Box::new(lhs.register(ctx)),
                Box::new(rhs.register(ctx)),
            ),

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(lhs.register(ctx)), rhs)
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new((*x).register(ctx)),
                args.into_iter()
                    .map(|x| x.register(ctx))
                    .collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key, value.register(ctx)))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new((*x).register(ctx))),
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Expression<usize, usize, usize> {
        match value {
            Expression::Primitive(x) => Expression::Primitive(x.clone()),

            Expression::Identifier(x) => Expression::Identifier(x.clone()),

            Expression::Group(x) => Expression::Group(Box::new(*x.node().id())),

            Expression::Closure(xs) => {
                Expression::Closure(xs.into_iter().map(|x| *x.node().id()).collect::<Vec<_>>())
            }

            Expression::UnaryOperation(op, x) => {
                Expression::UnaryOperation(op.clone(), Box::new(*x.node().id()))
            }

            Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                op.clone(),
                Box::new(*lhs.node().id()),
                Box::new(*rhs.node().id()),
            ),

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(*lhs.node().id()), rhs.clone())
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(*x.node().id()),
                args.into_iter().map(|x| *x.node().id()).collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key.clone(), *value.node().id()))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(*x.node().id())),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::expression::{
            binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive, statement::Statement,
            Expression, UnaryOperator,
        },
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn primitive() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::xc(Expression::Primitive(Primitive::Nil), ()).register(scope),
            f::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![(
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
            f::xc(Expression::Identifier(String::from("foo")), ()).register(scope),
            f::xc(
                Expression::Identifier(String::from("foo")),
                NodeContext::new(0, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![(
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
            f::xc(
                Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), ()))),
                (),
            )
            .register(scope),
            f::xc(
                Expression::Group(Box::new(f::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                ))),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::Closure(vec![
                    f::sc(
                        Statement::Variable(
                            String::from("foo"),
                            f::xc(Expression::Primitive(Primitive::Nil), ()),
                        ),
                        ()
                    ),
                    f::sc(
                        Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), ())),
                        ()
                    ),
                ]),
                (),
            )
            .register(scope),
            f::xc(
                Expression::Closure(vec![
                    f::sc(
                        Statement::Variable(
                            String::from("foo"),
                            f::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(0, vec![0, 1])
                            ),
                        ),
                        NodeContext::new(1, vec![0, 1])
                    ),
                    f::sc(
                        Statement::Effect(f::xc(
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
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            )
            .register(scope),
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            )
            .register(scope),
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    Box::new(f::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(1, vec![0])
                    )),
                ),
                NodeContext::new(2, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    String::from("foo"),
                ),
                (),
            )
            .register(scope),
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(
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
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::FunctionCall(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    vec![
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ],
                ),
                (),
            )
            .register(scope),
            f::xc(
                Expression::FunctionCall(
                    Box::new(f::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    vec![
                        f::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        ),
                        f::xc(
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
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ),
                    (
                        String::from("bar"),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ),
                ]),
                (),
            )
            .register(scope),
            f::xc(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        f::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(0, vec![0])
                        ),
                    ),
                    (
                        String::from("bar"),
                        f::xc(
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
            HashMap::from_iter(vec![
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
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), ()))),
                (),
            )
            .register(scope),
            f::xc(
                Expression::KSX(Box::new(f::kxc(
                    KSX::Text(String::from("foo")),
                    NodeContext::new(0, vec![0])
                ))),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (0, (vec![0], Fragment::KSX(KSX::Text(String::from("foo"))))),
                (
                    1,
                    (vec![0], Fragment::Expression(Expression::KSX(Box::new(0))))
                ),
            ])
        );
    }
}
