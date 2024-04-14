use super::storage;
use crate::{matcher as m, module};
use combine::{Parser, Stream};
use lang::ast;

pub fn module<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        storage::storage("module"),
        m::between(m::symbol('{'), m::symbol('}'), module::module()),
    )
        .map(|((storage, start), (value, end))| {
            ast::raw::Declaration::raw(ast::Declaration::module(storage, value), &start + &end)
        })
}
