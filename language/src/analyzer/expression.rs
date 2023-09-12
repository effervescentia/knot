use super::{Analyze, Context, Fragment};
use crate::parser::{
    expression::{ksx::KSXNode, statement::Statement, Expression, ExpressionNode},
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Analyze<ExpressionNode<T, usize>, Expression<usize, usize>> for ExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = Expression<ExpressionNode<T, C>, KSXNode<T, C>>;

    fn register(self, ctx: &mut Context) -> ExpressionNode<T, usize> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::Expression(Self::to_ref(&value));
        let id = ctx.register(fragment);

        ExpressionNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut Context) -> Self::Value<usize> {
        match value {
            Expression::Primitive(x) => Expression::Primitive(x),

            Expression::Identifier(x) => Expression::Identifier(x),

            Expression::Group(x) => Expression::Group(Box::new((*x).register(ctx))),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(x.register(ctx)),
                        Statement::Variable(name, x) => Statement::Variable(name, x.register(ctx)),
                    })
                    .collect::<Vec<_>>(),
            ),

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

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> Expression<usize, usize> {
        match value {
            Expression::Primitive(x) => Expression::Primitive(x.clone()),

            Expression::Identifier(x) => Expression::Identifier(x.clone()),

            Expression::Group(x) => Expression::Group(Box::new(x.node().id())),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(x.0.id()),
                        Statement::Variable(name, x) => Statement::Variable(name.clone(), x.0.id()),
                    })
                    .collect::<Vec<_>>(),
            ),

            Expression::UnaryOperation(op, x) => {
                Expression::UnaryOperation(op.clone(), Box::new(x.0.id()))
            }

            Expression::BinaryOperation(op, lhs, rhs) => {
                Expression::BinaryOperation(op.clone(), Box::new(lhs.0.id()), Box::new(rhs.0.id()))
            }

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(lhs.0.id()), rhs.clone())
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(x.0.id()),
                args.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key.clone(), value.0.id()))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(x.0.id())),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Analyze, Context, Fragment},
        parser::expression::{
            binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive, statement::Statement,
            Expression, UnaryOperator,
        },
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(Expression::Primitive(Primitive::Nil), ()).register(ctx),
            f::xc(Expression::Primitive(Primitive::Nil), 0)
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![(
                0,
                Fragment::Expression(Expression::Primitive(Primitive::Nil))
            )])
        );
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(Expression::Identifier(String::from("foo")), ()).register(ctx),
            f::xc(Expression::Identifier(String::from("foo")), 0)
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![(
                0,
                Fragment::Expression(Expression::Identifier(String::from("foo")))
            )])
        );
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), ()))),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0))),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (1, Fragment::Expression(Expression::Group(Box::new(0)))),
            ])
        );
    }

    #[test]
    fn closure() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ),
                    Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ]),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), 0),
                    ),
                    Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                ]),
                2,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Expression(Expression::Closure(vec![
                        Statement::Variable(String::from("foo"), 0),
                        Statement::Effect(1),
                    ]))
                ),
            ])
        );
    }

    #[test]
    fn unary_operation() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                ),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::UnaryOperation(
                        UnaryOperator::Not,
                        Box::new(0)
                    ))
                ),
            ])
        );
    }

    #[test]
    fn binary_operation() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                ),
                2,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Expression(Expression::BinaryOperation(
                        BinaryOperator::Equal,
                        Box::new(0),
                        Box::new(1),
                    ))
                ),
            ])
        );
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    String::from("foo"),
                ),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                    String::from("foo"),
                ),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::DotAccess(Box::new(0), String::from("foo")))
                ),
            ])
        );
    }

    #[test]
    fn function_call() {
        let ctx = &mut Context::new();

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
            .register(ctx),
            f::xc(
                Expression::FunctionCall(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                    vec![
                        f::xc(Expression::Primitive(Primitive::Nil), 1),
                        f::xc(Expression::Primitive(Primitive::Nil), 2),
                    ],
                ),
                3,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    3,
                    Fragment::Expression(Expression::FunctionCall(Box::new(0), vec![1, 2]))
                ),
            ])
        );
    }

    #[test]
    fn style() {
        let ctx = &mut Context::new();

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
            .register(ctx),
            f::xc(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), 0),
                    ),
                    (
                        String::from("bar"),
                        f::xc(Expression::Primitive(Primitive::Nil), 1),
                    ),
                ]),
                2,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Expression(Expression::Style(vec![
                        (String::from("foo"), 0),
                        (String::from("bar"), 1),
                    ]))
                ),
            ])
        );
    }

    #[test]
    fn ksx() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), ()))),
                (),
            )
            .register(ctx),
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), 0))),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::KSX(KSX::Text(String::from("foo")))),
                (1, Fragment::Expression(Expression::KSX(Box::new(0)))),
            ])
        );
    }
}
