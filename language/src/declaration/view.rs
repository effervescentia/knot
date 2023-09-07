use super::{parameter, storage, Declaration, DeclarationRaw};
use crate::{expression, matcher as m, position::Decrement};
use combine::{between, optional, sep_end_by, Parser, Stream};
use std::fmt::Debug;

// view foo -> nil;
// view foo -> {};
// view foo -> { nil; };
// view foo: nil -> nil;
// view foo() -> nil;
// view foo(): nil -> nil;
// view foo(props) -> nil;
// view foo({a, b: nil, c = 123}) -> nil;

pub fn view<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::terminated((
        storage::storage("view"),
        optional(between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by(parameter::parameter(), m::symbol(',')),
        )),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(|((name, start), attributes, _, body)| {
        let range = start.concat(body.get_range());
        DeclarationRaw(
            Declaration::View {
                name,
                parameters: attributes.unwrap_or(vec![]),
                body,
            },
            range,
        )
    })
}
