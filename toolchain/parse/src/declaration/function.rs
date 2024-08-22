use super::{parameter, storage};
use crate::{expression, matcher as m, types::typedef};
use combine::{optional, Parser, Stream};
use lang::ast;

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
        m::lambda(
            (
                optional(m::tuple(parameter::parameter())),
                typedef::typedef(),
            ),
            expression::expression(),
        ),
    ))
    .map(|((storage, start), ((parameters, body_type), body))| {
        let range = &start + body.0.range();

        ast::raw::Declaration::raw(
            ast::Declaration::function(
                storage,
                parameters.map(|x| x.0).unwrap_or_default(),
                body_type,
                body,
            ),
            range,
        )
    })
}
