use super::{parameter, storage};
use crate::{ast, expression, matcher as m};
use combine::{between, optional, sep_end_by, Parser, Stream};

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
        optional(between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by(parameter::parameter(), m::symbol(',')),
        )),
        m::glyph("->"),
        expression::expression(),
    ))
    .map(|((storage, start), attributes, _, body)| {
        let range = &start + body.0.range();
        ast::raw::Declaration::new(
            ast::Declaration::view(storage, attributes.unwrap_or_default(), body),
            range,
        )
    })
}
