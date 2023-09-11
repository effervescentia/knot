use super::{ksx, Context};
use crate::parser::{
    expression::{statement::Statement, Expression, ExpressionNode},
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_expression<T>(
    x: ExpressionNode<T, ()>,
    ctx: &mut Context,
) -> ExpressionNode<T, usize>
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
        parser::expression::{
            binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive, statement::Statement,
            Expression, UnaryOperator,
        },
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        let result = analyze_expression(f::xc(Expression::Primitive(Primitive::Nil), ()), ctx);

        assert_eq!(result, f::xc(Expression::Primitive(Primitive::Nil), 0))
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        let result =
            analyze_expression(f::xc(Expression::Identifier(String::from("foo")), ()), ctx);

        assert_eq!(
            result,
            f::xc(Expression::Identifier(String::from("foo")), 0)
        )
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        let result = analyze_expression(
            f::xc(
                Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), ()))),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::Closure(vec![
                    Statement::Variable(
                        String::from("foo"),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ),
                    Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ]),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::DotAccess(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    String::from("foo"),
                ),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::FunctionCall(
                    Box::new(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    vec![
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                        f::xc(Expression::Primitive(Primitive::Nil), ()),
                    ],
                ),
                (),
            ),
            ctx,
        );

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

        let result = analyze_expression(
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
            ),
            ctx,
        );

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

        let result = analyze_expression(
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), ()))),
                (),
            ),
            ctx,
        );

        assert_eq!(
            result,
            f::xc(
                Expression::KSX(Box::new(f::kxc(KSX::Text(String::from("foo")), 0,))),
                1,
            )
        )
    }
}
