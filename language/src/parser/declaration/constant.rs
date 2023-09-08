use super::{storage, Declaration, DeclarationRaw};
use crate::parser::{
    expression,
    range::Ranged,
    {matcher as m, position::Decrement, types::typedef},
};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn constant<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
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
        let range = &start + value.range();
        DeclarationRaw(
            Declaration::Constant {
                name,
                value_type,
                value,
            },
            range,
        )
    })
}
