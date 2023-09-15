use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        position::Decrement,
        types::type_expression::{self, TypeExpression},
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for type_expression::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::TypeExpression(match self {
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
        })
    }
}
