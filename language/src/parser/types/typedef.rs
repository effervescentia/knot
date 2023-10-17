use super::type_expression;
use crate::{ast::TypeExpressionNode, common::position::Position, parser::matcher as m};
use combine::{optional, Parser, Stream};

pub fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpressionNode<()>>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}
