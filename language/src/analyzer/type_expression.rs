use super::{Analyze, Context, Fragment};
use crate::parser::{
    node::Node,
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Analyze<TypeExpressionNode<T, usize>, TypeExpression<usize>> for TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = TypeExpression<TypeExpressionNode<T, C>>;

    fn register(self, ctx: &mut Context) -> TypeExpressionNode<T, usize> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::TypeExpression(Self::to_ref(&value));
        let id = ctx.register(fragment);

        TypeExpressionNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut Context) -> Self::Value<usize> {
        match value {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new((*x).register(ctx))),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new((*lhs).register(ctx)), rhs)
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params
                    .into_iter()
                    .map(|x| x.register(ctx))
                    .collect::<Vec<_>>(),
                Box::new((*body).register(ctx)),
            ),
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> TypeExpression<usize> {
        match value {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x.clone()),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new((*x).0.id())),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new((*lhs).0.id()), rhs.clone())
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
                Box::new(body.0.id()),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Analyze, Context, Fragment},
        parser::types::type_expression::TypeExpression,
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::txc(TypeExpression::Nil, ()).register(ctx),
            f::txc(TypeExpression::Nil, 0)
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![(0, Fragment::TypeExpression(TypeExpression::Nil))])
        );
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::txc(TypeExpression::Identifier(String::from("foo")), ()).register(ctx),
            f::txc(TypeExpression::Identifier(String::from("foo")), 0)
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![(
                0,
                Fragment::TypeExpression(TypeExpression::Identifier(String::from("foo")))
            )])
        );
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::txc(
                TypeExpression::Group(Box::new(f::txc(TypeExpression::Nil, ()))),
                (),
            )
            .register(ctx),
            f::txc(
                TypeExpression::Group(Box::new(f::txc(TypeExpression::Nil, 0))),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::TypeExpression(TypeExpression::Group(Box::new(0)))
                )
            ])
        );
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, ())),
                    String::from("foo"),
                ),
                (),
            )
            .register(ctx),
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, 0)),
                    String::from("foo"),
                ),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::TypeExpression(TypeExpression::DotAccess(
                        Box::new(0),
                        String::from("foo"),
                    ))
                )
            ])
        );
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::txc(
                TypeExpression::Function(
                    vec![
                        f::txc(TypeExpression::Nil, ()),
                        f::txc(TypeExpression::Nil, ()),
                    ],
                    Box::new(f::txc(TypeExpression::Nil, ())),
                ),
                (),
            )
            .register(ctx),
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
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (1, Fragment::TypeExpression(TypeExpression::Nil)),
                (2, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    3,
                    Fragment::TypeExpression(TypeExpression::Function(vec![0, 1], Box::new(2)))
                )
            ])
        );
    }
}
