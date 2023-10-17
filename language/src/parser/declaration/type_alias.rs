use super::storage;
use crate::{
    ast::{Declaration, DeclarationNode},
    common::position::Position,
    parser::{matcher as m, types::type_expression},
};
use combine::{Parser, Stream};

pub fn type_alias<T>() -> impl Parser<T, Output = DeclarationNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::terminated((
        storage::storage("type"),
        m::symbol('='),
        type_expression::type_expression(),
    ))
    .map(|((name, start), _, value)| {
        let range = &start + value.node().range();
        DeclarationNode::raw(Declaration::TypeAlias { name, value }, range)
    })
}
