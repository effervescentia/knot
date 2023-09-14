use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
use crate::parser::{
    node::Node,
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Analyze for TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Ref = TypeExpression<usize>;
    type Node = TypeExpressionNode<T, NodeContext>;
    type Value<C> = TypeExpression<TypeExpressionNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::TypeExpression(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        TypeExpressionNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
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

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Self::Ref {
        match value {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x.clone()),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new(*(*x).0.id())),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new(*(*lhs).0.id()), rhs.clone())
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>(),
                Box::new(*body.0.id()),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::types::type_expression::TypeExpression,
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn primitive() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::txc(TypeExpression::Nil, ()).register(scope),
            f::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![(
                0,
                (vec![0], Fragment::TypeExpression(TypeExpression::Nil))
            )])
        );
    }

    #[test]
    fn identifier() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::txc(TypeExpression::Identifier(String::from("foo")), ()).register(scope),
            f::txc(
                TypeExpression::Identifier(String::from("foo")),
                NodeContext::new(0, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![(
                0,
                (
                    vec![0],
                    Fragment::TypeExpression(TypeExpression::Identifier(String::from("foo")))
                )
            )])
        );
    }

    #[test]
    fn group() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::txc(
                TypeExpression::Group(Box::new(f::txc(TypeExpression::Nil, ()))),
                (),
            )
            .register(scope),
            f::txc(
                TypeExpression::Group(Box::new(f::txc(
                    TypeExpression::Nil,
                    NodeContext::new(0, vec![0])
                ))),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (0, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (
                    1,
                    (
                        vec![0],
                        Fragment::TypeExpression(TypeExpression::Group(Box::new(0)))
                    )
                )
            ])
        );
    }

    #[test]
    fn dot_access() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, ())),
                    String::from("foo"),
                ),
                (),
            )
            .register(scope),
            f::txc(
                TypeExpression::DotAccess(
                    Box::new(f::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                    String::from("foo"),
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (0, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (
                    1,
                    (
                        vec![0],
                        Fragment::TypeExpression(TypeExpression::DotAccess(
                            Box::new(0),
                            String::from("foo"),
                        ))
                    )
                )
            ])
        );
    }

    #[test]
    fn function() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
            .register(scope),
            f::txc(
                TypeExpression::Function(
                    vec![
                        f::txc(TypeExpression::Nil, NodeContext::new(0, vec![0])),
                        f::txc(TypeExpression::Nil, NodeContext::new(1, vec![0])),
                    ],
                    Box::new(f::txc(TypeExpression::Nil, NodeContext::new(2, vec![0]))),
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (0, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (1, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (2, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (
                    3,
                    (
                        vec![0],
                        Fragment::TypeExpression(TypeExpression::Function(vec![0, 1], Box::new(2)))
                    )
                )
            ])
        );
    }
}
