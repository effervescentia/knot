use super::{storage, Declaration, DeclarationNode};
use crate::parser::{
    expression,
    range::Ranged,
    {matcher as m, position::Decrement, types::typedef},
};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn constant<T>() -> impl Parser<T, Output = DeclarationNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::terminated((
        storage::storage("const"),
        typedef::typedef(),
        m::symbol('='),
        expression::expression(),
    ))
    .map(|((name, start), value_type, _, value)| {
        let range = &start + value.0.range();

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
