use super::type_expression;
use crate::{ast::TypeExpressionNode, common::position::Decrement, parser::matcher as m};
use combine::{optional, Parser, Stream};
use std::fmt::Debug;

pub fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpressionNode<T, ()>>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}
