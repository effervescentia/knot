use super::storage;
use crate::{matcher as m, types::type_expression, Position, Range};
use combine::{Parser, Stream};
use lang::ast::{AstNode, Declaration, DeclarationNode};

pub fn type_alias<T>() -> impl Parser<T, Output = DeclarationNode<Range, ()>>
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
