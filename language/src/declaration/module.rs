use super::{storage, Declaration, DeclarationRaw};
use crate::{matcher as m, module, position::Decrement};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn module<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        storage::storage("module"),
        m::between(m::symbol('{'), m::symbol('}'), module::module()),
    )
        .map(|((name, start), (value, end))| {
            DeclarationRaw(Declaration::Module { name, value }, &start + &end)
        })
}
