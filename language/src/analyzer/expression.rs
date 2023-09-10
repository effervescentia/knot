use super::{ksx, Context};
use crate::parser::{
    expression::{statement::Statement, Expression, ExpressionNode},
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_expression<T>(x: ExpressionNode<T, ()>, ctx: &mut Context) -> ExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    ExpressionNode(
        x.0.map(|x| match x {
            Expression::Primitive(x) => Expression::Primitive(x),

            Expression::Identifier(x) => Expression::Identifier(x),

            Expression::Group(x) => Expression::Group(Box::new(analyze_expression(*x, ctx))),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(analyze_expression(x, ctx)),
                        Statement::Variable(name, x) => {
                            Statement::Variable(name, analyze_expression(x, ctx))
                        }
                    })
                    .collect::<Vec<_>>(),
            ),

            Expression::UnaryOperation(op, lhs) => {
                Expression::UnaryOperation(op, Box::new(analyze_expression(*lhs, ctx)))
            }

            Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                op,
                Box::new(analyze_expression(*lhs, ctx)),
                Box::new(analyze_expression(*rhs, ctx)),
            ),

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(analyze_expression(*lhs, ctx)), rhs)
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(analyze_expression(*x, ctx)),
                args.into_iter()
                    .map(|x| analyze_expression(x, ctx))
                    .collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key, analyze_expression(value, ctx)))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(ksx::analyze_ksx(*x, ctx))),
        })
        .with_context(ctx.generate_id()),
    )
}

#[cfg(test)]
mod tests {
    use super::analyze_expression;
    use crate::{
        analyzer::Context,
        parser::{
            expression::{
                binary_operation::BinaryOperator,
                ksx::{KSXNode, KSX},
                primitive::Primitive,
                statement::Statement,
                Expression, ExpressionNode, UnaryOperator,
            },
            node::Node,
            range::Range,
            CharStream,
        },
    };

    const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0))
        )
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(Expression::Identifier(String::from("foo")), RANGE, ())),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(Expression::Identifier(String::from("foo")), RANGE, 0))
        )
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::Group(Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    (),
                )))),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::Group(Box::new(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                )))),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn closure() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                    ),
                    Statement::Effect(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                ]),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0)),
                    ),
                    Statement::Effect(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        1,
                    ))),
                ]),
                RANGE,
                2,
            ))
        )
    }

    #[test]
    fn unary_operation() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        0,
                    ))),
                ),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn binary_operation() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        0,
                    ))),
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        1,
                    ))),
                ),
                RANGE,
                2,
            ))
        )
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::DotAccess(
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                    String::from("foo"),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::DotAccess(
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        0,
                    ))),
                    String::from("foo"),
                ),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn function_call() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::FunctionCall(
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                    vec![
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                    ],
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::FunctionCall(
                    Box::new(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        0,
                    ))),
                    vec![
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 2)),
                    ],
                ),
                RANGE,
                3,
            ))
        )
    }

    #[test]
    fn style() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                    ),
                    (
                        String::from("bar"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, ())),
                    ),
                ]),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::Style(vec![
                    (
                        String::from("foo"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 0)),
                    ),
                    (
                        String::from("bar"),
                        ExpressionNode(Node(Expression::Primitive(Primitive::Nil), RANGE, 1)),
                    ),
                ]),
                RANGE,
                2,
            ))
        )
    }

    #[test]
    fn ksx() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            ExpressionNode(Node(
                Expression::KSX(Box::new(KSXNode(Node(
                    KSX::Text(String::from("foo")),
                    RANGE,
                    (),
                )))),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            ExpressionNode(Node(
                Expression::KSX(Box::new(KSXNode(Node(
                    KSX::Text(String::from("foo")),
                    RANGE,
                    0,
                )))),
                RANGE,
                1,
            ))
        )
    }
}
