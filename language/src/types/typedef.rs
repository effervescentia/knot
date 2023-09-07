use super::type_expression::{self, TypeExpression};
use crate::{matcher as m, position::Decrement};
use combine::{optional, Parser, Stream};
use std::fmt::Debug;

pub fn typedef<T>() -> impl Parser<T, Output = Option<TypeExpression>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    optional(m::symbol(':').with(type_expression::type_expression()))
}