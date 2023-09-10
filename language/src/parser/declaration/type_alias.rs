use super::{storage, Declaration, DeclarationNode};
use crate::parser::{matcher as m, position::Decrement, range::Ranged, types::type_expression};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn type_alias<T>() -> impl Parser<T, Output = DeclarationNode<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::terminated((
        storage::storage("type"),
        m::symbol('='),
        type_expression::type_expression(),
    ))
    .map(|((name, start), _, value)| {
        let range = &start + value.range();
        DeclarationNode(Declaration::TypeAlias { name, value }, range)
    })
}
