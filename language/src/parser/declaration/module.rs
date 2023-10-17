use super::storage;
use crate::{
    ast::{Declaration, DeclarationNode},
    common::position::Position,
    parser::{matcher as m, module},
};
use combine::{Parser, Stream};

pub fn module<T>() -> impl Parser<T, Output = DeclarationNode<()>>
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
