use super::{parameter, storage};
use crate::{ast, expression, matcher as m, types::typedef};
use combine::{between, optional, sep_end_by, Parser, Stream};

// func foo -> nil;
// func foo -> {};
// func foo -> { nil; };
// func foo: nil -> nil;
// func foo() -> nil;
// func foo(): nil -> nil;
// func foo(a, b: nil, c = 123) -> nil;

pub fn function<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
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
    .map(|((storage, start), parameters, body_type, _, body)| {
        let range = &start + body.0.range();

        ast::raw::Declaration::new(
            ast::Declaration::function(storage, parameters.unwrap_or_default(), body_type, body),
            range,
        )
    })
}