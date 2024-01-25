use super::type_expression;
use crate::{ast, matcher as m};
use combine::{optional, Parser, Stream};

pub fn typedef<T>() -> impl Parser<T, Output = Option<ast::raw::TypeExpression>>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}
