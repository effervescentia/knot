use super::{parameter, storage, Declaration, DeclarationRaw};
use crate::{expression, matcher as m, position::Decrement, types::typedef};
use combine::{between, optional, sep_end_by, Parser, Stream};
use std::fmt::Debug;

// func foo -> nil;
// func foo -> {};
// func foo -> { nil; };
// func foo: nil -> nil;
// func foo() -> nil;
// func foo(): nil -> nil;
// func foo(a, b: nil, c = 123) -> nil;

pub fn function<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::terminated((
        storage::storage("func"),
        optional(between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by(parameter::parameter(), m::symbol(',')),
        )),
        typedef::typedef(),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(|((name, start), parameters, body_type, _, body)| {
        let range = start.concat(body.get_range());
        DeclarationRaw(
            Declaration::Function {
                name,
                parameters: parameters.unwrap_or(vec![]),
                body_type,
                body,
            },
            range,
        )
    })
}
