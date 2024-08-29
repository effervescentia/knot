use super::{parameter, storage};
use crate::{expression, matcher as m};
use combine::{optional, sep_end_by, Parser, Stream};
use lang::ast;

// view foo -> nil;
// view foo -> {};
// view foo -> { nil; };
// view foo {} -> nil;
// view foo { a, b: nil, c = 123 } -> nil;

pub fn view<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("view"),
        m::lambda(
            optional(m::closure(sep_end_by(
                parameter::parameter(),
                m::symbol(','),
            ))),
            expression::expression(),
        ),
    ))
    .map(|((storage, start), (attributes, body))| {
        let range = &start + body.0.range();
        ast::raw::Declaration::raw(
            ast::Declaration::view(storage, attributes.map(|x| x.0).unwrap_or_default(), body),
            range,
        )
    })
}
