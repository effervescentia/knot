use super::storage;
use crate::{matcher as m, module, Position, Range};
use combine::{Parser, Stream};
use lang::ast::{Declaration, DeclarationNode};

pub fn module<T>() -> impl Parser<T, Output = DeclarationNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    (
        storage::storage("module"),
        m::between(m::symbol('{'), m::symbol('}'), module::module()),
    )
        .map(|((name, start), (value, end))| {
            DeclarationNode::raw(Declaration::Module { name, value }, &start + &end)
        })
}
