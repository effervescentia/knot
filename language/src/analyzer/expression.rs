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
        analyzer::{Analyze, Context},
        parser::expression::{
            binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive, statement::Statement,
            Expression, UnaryOperator,
        },
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        let result = f::xc(Expression::Primitive(Primitive::Nil), ()).register(ctx);

        assert_eq!(result, f::xc(Expression::Primitive(Primitive::Nil), 0))
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        let result = f::xc(Expression::Identifier(String::from("foo")), ()).register(ctx);

        assert_eq!(
            result,
            f::xc(Expression::Identifier(String::from("foo")), 0)
        )
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), ()))),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0,))),
                1,
            )
        )
    }

    #[test]
    fn closure() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::Closure(vec![
                Statement::Variable(
                    String::from("foo"),
                    f::xc(Expression::Primitive(Primitive::Nil), ()),
                ),
                Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), ())),
            ]),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), 0),
                    ),
                    Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), 1,)),
                ]),
                2,
            )
        )
    }

    #[test]
    fn unary_operation() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
            ),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                ),
                1,
            )
        )
    }

    #[test]
    fn binary_operation() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
            ),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 1,)),
                ),
                2,
            )
        )
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::DotAccess(
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                String::from("foo"),
            ),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                    String::from("foo"),
                ),
                1,
            )
        )
    }

    #[test]
    fn function_call() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::FunctionCall(
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                vec![
                    f::xc(Expression::Primitive(Primitive::Nil), ()),
                    f::xc(Expression::Primitive(Primitive::Nil), ()),
                ],
            ),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
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
        )
    }

    #[test]
    fn style() {
        let ctx = &mut Context::new();

        let result = f::xc(
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
        .register(ctx);

        assert_eq!(
            result,
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
        )
    }

    #[test]
    fn ksx() {
        let ctx = &mut Context::new();

        let result = f::xc(
            Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), ()))),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), 0,))),
                1,
            )
        )
    }

    // mod to_ref {
    //     use super::*;

    //     #[test]
    //     fn primitive() {
    //         let input = f::xc(Expression::Primitive(Primitive::Nil), 0);

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::Primitive(Primitive::Nil)
    //         )
    //     }

    //     #[test]
    //     fn identifier() {
    //         let input = f::xc(Expression::Identifier(String::from("foo")), 0);

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::Identifier(String::from("foo"))
    //         )
    //     }

    //     #[test]
    //     fn group() {
    //         let input = f::xc(
    //             Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0))),
    //             1,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::Group(Box::new(0))
    //         )
    //     }

    //     #[test]
    //     fn closure() {
    //         let input = f::xc(
    //             Expression::Closure(vec![
    //                 Statement::Variable(
    //                     String::from("foo"),
    //                     f::xc(Expression::Primitive(Primitive::Nil), 0),
    //                 ),
    //                 Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), 1)),
    //             ]),
    //             2,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::Closure(vec![
    //                 Statement::Variable(String::from("foo"), 0),
    //                 Statement::Effect(1),
    //             ])
    //         )
    //     }

    //     #[test]
    //     fn unary_operation() {
    //         let input = f::xc(
    //             Expression::UnaryOperation(
    //                 UnaryOperator::Not,
    //                 Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
    //             ),
    //             1,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::UnaryOperation(UnaryOperator::Not, Box::new(0))
    //         )
    //     }

    //     #[test]
    //     fn binary_operation() {
    //         let input: ExpressionNode<
    //             combine::easy::Stream<
    //                 combine::stream::position::Stream<
    //                     &str,
    //                     combine::stream::position::SourcePosition,
    //                 >,
    //             >,
    //             usize,
    //         > = f::xc(
    //             Expression::BinaryOperation(
    //                 BinaryOperator::Equal,
    //                 Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
    //                 Box::new(f::xc(Expression::Primitive(Primitive::Nil), 1)),
    //             ),
    //             2,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::BinaryOperation(BinaryOperator::Equal, Box::new(0), Box::new(1))
    //         )
    //     }

    //     #[test]
    //     fn dot_access() {
    //         let input = f::xc(
    //             Expression::DotAccess(
    //                 Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
    //                 String::from("foo"),
    //             ),
    //             1,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::DotAccess(Box::new(0), String::from("foo"))
    //         )
    //     }

    //     #[test]
    //     fn function_call() {
    //         let input = f::xc(
    //             Expression::FunctionCall(
    //                 Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
    //                 vec![
    //                     f::xc(Expression::Primitive(Primitive::Nil), 1),
    //                     f::xc(Expression::Primitive(Primitive::Nil), 2),
    //                 ],
    //             ),
    //             3,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::FunctionCall(Box::new(0), vec![1, 2])
    //         )
    //     }

    //     #[test]
    //     fn style() {
    //         let input = f::xc(
    //             Expression::Style(vec![
    //                 (
    //                     String::from("foo"),
    //                     f::xc(Expression::Primitive(Primitive::Nil), 0),
    //                 ),
    //                 (
    //                     String::from("bar"),
    //                     f::xc(Expression::Primitive(Primitive::Nil), 1),
    //                 ),
    //             ]),
    //             2,
    //         );

    //         assert_eq!(
    //             input.node().value().to_ref(),
    //             Expression::Style(vec![(String::from("foo"), 0), (String::from("bar"), 1)])
    //         )
    //     }

    //     #[test]
    //     fn ksx() {
    //         let input = f::xc(
    //             Expression::KSX(Box::new(f::kxc(KSX::Fragment(vec![]), 0))),
    //             1,
    //         );

    //         assert_eq!(input.node().value().to_ref(), Expression::KSX(Box::new(0)))
    //     }
    // }
}
