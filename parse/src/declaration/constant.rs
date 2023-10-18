use super::storage;
use crate::{
    expression, Position, Range,
    {matcher as m, types::typedef},
};
use combine::{Parser, Stream};
use lang::ast::{Declaration, DeclarationNode};

pub fn constant<T>() -> impl Parser<T, Output = DeclarationNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::terminated((
        storage::storage("const"),
        typedef::typedef(),
        m::symbol('='),
        expression::expression(),
    ))
    .map(|((name, start), value_type, _, value)| {
        let range = &start + value.node().range();

        DeclarationNode::raw(
            Declaration::Constant {
                name,
                value_type,
                value,
            },
            range,
        )
    })
}
