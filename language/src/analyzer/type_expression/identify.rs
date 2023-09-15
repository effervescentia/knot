use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    parser::{
        position::Decrement,
        types::type_expression::{self, TypeExpression},
    },
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
