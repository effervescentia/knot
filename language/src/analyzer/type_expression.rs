use super::{reference::ToRef, Context, Fragment, Register};
use crate::parser::{
    node::Node,
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_type_expression<T>(
    x: TypeExpressionNode<T, ()>,
    ctx: &mut Context,
) -> TypeExpressionNode<T, usize>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    TypeExpressionNode(
        x.0.map(|x| match x {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

            TypeExpression::Group(x) => {
                TypeExpression::Group(Box::new(analyze_type_expression(*x, ctx)))
            }

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new(analyze_type_expression(*lhs, ctx)), rhs)
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params
                    .into_iter()
                    .map(|x| analyze_type_expression(x, ctx))
                    .collect::<Vec<_>>(),
                Box::new(analyze_type_expression(*body, ctx)),
            ),
        })
        .with_context(ctx.generate_id()),
    )
}

fn identify_type_expression<T>(
    x: TypeExpressionNode<T, ()>,
    ctx: &mut Context,
) -> Node<TypeExpression<TypeExpressionNode<T, usize>>, T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.0.map(|x| match x {
        TypeExpression::Nil => TypeExpression::Nil,
        TypeExpression::Boolean => TypeExpression::Boolean,
        TypeExpression::Integer => TypeExpression::Integer,
        TypeExpression::Float => TypeExpression::Float,
        TypeExpression::String => TypeExpression::String,
        TypeExpression::Style => TypeExpression::Style,
        TypeExpression::Element => TypeExpression::Element,

        TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

        TypeExpression::Group(x) => {
            TypeExpression::Group(Box::new(analyze_type_expression(*x, ctx)))
        }

        TypeExpression::DotAccess(lhs, rhs) => {
            TypeExpression::DotAccess(Box::new(analyze_type_expression(*lhs, ctx)), rhs)
        }

        TypeExpression::Function(params, body) => TypeExpression::Function(
            params
                .into_iter()
                .map(|x| analyze_type_expression(x, ctx))
                .collect::<Vec<_>>(),
            Box::new(analyze_type_expression(*body, ctx)),
        ),
    })
}

impl<T> Register<TypeExpressionNode<T, usize>> for TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn register(self, ctx: &mut Context) -> TypeExpressionNode<T, usize> {
        let node = identify_type_expression(self, ctx);
        let fragment = Fragment::TypeExpression(node.value().to_ref());
        let id = ctx.register(fragment);

        TypeExpressionNode(node.with_context(id))
    }
}

#[cfg(test)]
mod tests {
    use super::analyze_type_expression;
    use crate::{
        analyzer::Context, parser::types::type_expression::TypeExpression, test::fixture as f,
    };

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(f::txc(TypeExpression::Nil, ()), ctx);

        assert_eq!(result, f::txc(TypeExpression::Nil, 0))
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            f::txc(TypeExpression::Identifier(String::from("foo")), ()),
            ctx,
        );

        assert_eq!(
            result,
            f::txc(TypeExpression::Identifier(String::from("foo")), 0)
        )
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            f::txc(
                TypeExpression::Group(Box::new(f::txc(TypeExpression::Nil, ()))),
                (),
            ),
            ctx,
        );

        assert_eq!(
            result,
            f::txc(
                TypeExpression::Group(Box::new(f::txc(TypeExpression::Nil, 0,))),
                1,
            )
        )
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, ())),
                    String::from("foo"),
                ),
                (),
            ),
            ctx,
        );

        assert_eq!(
            result,
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, 0)),
                    String::from("foo"),
                ),
                1,
            )
        )
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            f::txc(
                TypeExpression::Function(
                    vec![
                        f::txc(TypeExpression::Nil, ()),
                        f::txc(TypeExpression::Nil, ()),
                    ],
                    Box::new(f::txc(TypeExpression::Nil, ())),
                ),
                (),
            ),
            ctx,
        );

        assert_eq!(
            result,
            f::txc(
                TypeExpression::Function(
                    vec![
                        f::txc(TypeExpression::Nil, 0),
                        f::txc(TypeExpression::Nil, 1),
                    ],
                    Box::new(f::txc(TypeExpression::Nil, 2)),
                ),
                3,
            )
        )
    }
}
