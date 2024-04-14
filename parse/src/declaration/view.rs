use super::{parameter, storage};
use crate::{expression, matcher as m};
use combine::{optional, Parser, Stream};
use lang::ast;

// view foo -> nil;
// view foo -> {};
// view foo -> { nil; };
// view foo: nil -> nil;
// view foo() -> nil;
// view foo(): nil -> nil;
// view foo(props) -> nil;
// view foo({a, b: nil, c = 123}) -> nil;

pub fn view<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("view"),
        optional(m::tuple(parameter::parameter())),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(|((storage, start), attributes, _, body)| {
        let range = &start + body.0.range();
        ast::raw::Declaration::raw(
            ast::Declaration::view(storage, attributes.map(|x| x.0).unwrap_or_default(), body),
            range,
        )
    })
}
