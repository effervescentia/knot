use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::type_expression::{self, TypeExpression},
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<type_expression::NodeValue<T, NodeContext>> for type_expression::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> type_expression::NodeValue<T, NodeContext> {
        match self {
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
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::type_expression::{TypeExpression, TypeExpressionNode},
        parser::CharStream,
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<CharStream<'static>, ()>>::Nil.identify(scope),
            TypeExpression::Nil
        );
    }

    #[test]
    fn identifier() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<CharStream<'static>, ()>>::Identifier(
                String::from("foo")
            )
            .identify(scope),
            TypeExpression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn group() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            TypeExpression::Group(Box::new(f::n::tx(TypeExpression::Nil))).identify(scope),
            TypeExpression::Group(Box::new(f::n::txc(
                TypeExpression::Nil,
                NodeContext::new(0, vec![0])
            )))
        );
    }

    #[test]
    fn dot_access() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            TypeExpression::DotAccess(
                Box::new(f::n::tx(TypeExpression::Nil)),
                String::from("foo"),
            )
            .identify(scope),
            TypeExpression::DotAccess(
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                String::from("foo"),
            )
        );
    }

    #[test]
    fn function() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            TypeExpression::Function(
                vec![f::n::tx(TypeExpression::Nil), f::n::tx(TypeExpression::Nil),],
                Box::new(f::n::tx(TypeExpression::Nil)),
            )
            .identify(scope),
            TypeExpression::Function(
                vec![
                    f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0])),
                    f::n::txc(TypeExpression::Nil, NodeContext::new(1, vec![0])),
                ],
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(2, vec![0]))),
            )
        );
    }
}
