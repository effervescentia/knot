use super::{reference::ToRef, Context, Fragment, Register};
use crate::parser::{
    expression::{ksx::KSXNode, statement::Statement, Expression, ExpressionNode},
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

fn identify_expression<T>(
    x: ExpressionNode<T, ()>,
    ctx: &mut Context,
) -> Node<Expression<ExpressionNode<T, usize>, KSXNode<T, usize>>, T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.0.map(|x| match x {
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

        Expression::DotAccess(lhs, rhs) => Expression::DotAccess(Box::new(lhs.register(ctx)), rhs),

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
    })
}

impl<T> Register<ExpressionNode<T, usize>> for ExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn register(self, ctx: &mut Context) -> ExpressionNode<T, usize> {
        let node = identify_expression(self, ctx);
        let fragment = Fragment::Expression(node.value().to_ref());
        let id = ctx.register(fragment);

        ExpressionNode(node.with_context(id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Context, Register},
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
}
