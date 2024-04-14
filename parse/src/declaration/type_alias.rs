use super::storage;
use crate::{matcher as m, types::type_expression};
use combine::{Parser, Stream};
use lang::ast;

pub fn type_alias<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("type"),
        m::symbol('='),
        type_expression::type_expression(),
    ))
    .map(|((storage, start), _, value)| {
        let range = &start + value.0.range();

        ast::raw::Declaration::raw(ast::Declaration::type_alias(storage, value), range)
    })
}
