use crate::{
    expression::{self, ExpressionRaw},
    matcher as m,
    types::{type_expression::TypeExpression, typedef},
};
use combine::{optional, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Parameter<E> {
    pub name: String,
    pub value_type: Option<TypeExpression>,
    pub default_value: Option<E>,
}

pub type ParameterRaw<T> = Parameter<ExpressionRaw<T>>;

pub fn parameter<T>() -> impl Parser<T, Output = ParameterRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    (
        m::standard_identifier(),
        typedef::typedef(),
        optional(m::symbol('=').with(expression::expression())),
    )
        .map(|((name, _), value_type, default_value)| ParameterRaw {
            name,
            value_type,
            default_value,
        })
}
