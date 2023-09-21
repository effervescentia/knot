use super::storage;
use crate::{
    ast::declaration::{Declaration, DeclarationNode},
    common::position::Decrement,
    parser::{matcher as m, module},
};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn module<T>() -> impl Parser<T, Output = DeclarationNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        storage::storage("module"),
        m::between(m::symbol('{'), m::symbol('}'), module::module()),
    )
        .map(|((name, start), (value, end))| {
            DeclarationNode::raw(Declaration::Module { name, value }, &start + &end)
        })
}
