use super::storage;
use crate::{
    expression,
    {matcher as m, types::typedef},
};
use combine::{Parser, Stream};
use lang::ast;

pub fn constant<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("const"),
        typedef::typedef(),
        m::symbol('='),
        expression::expression(),
    ))
    .map(|((storage, start), value_type, _, value)| {
        let range = &start + value.0.range();

        ast::raw::Declaration::raw(
            ast::Declaration::constant(storage, value_type, value),
            range,
        )
    })
}
