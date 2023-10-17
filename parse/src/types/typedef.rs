use super::type_expression;
use crate::matcher as m;
use combine::{optional, Parser, Stream};
use lang::{ast::TypeExpressionNode, Position};

pub fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpressionNode<()>>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}
