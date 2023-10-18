use super::type_expression;
use crate::{matcher as m, Position, Range};
use combine::{optional, Parser, Stream};
use lang::ast::TypeExpressionNode;

pub fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpressionNode<Range, ()>>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}
