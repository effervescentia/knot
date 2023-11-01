use super::{parameter, storage};
use crate::{expression, matcher as m, types::typedef, Position, Range};
use combine::{between, optional, sep_end_by, Parser, Stream};
use lang::ast::{Declaration, DeclarationNode};

// func foo -> nil;
// func foo -> {};
// func foo -> { nil; };
// func foo: nil -> nil;
// func foo() -> nil;
// func foo(): nil -> nil;
// func foo(a, b: nil, c = 123) -> nil;

pub fn function<T>() -> impl Parser<T, Output = DeclarationNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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
        let range = &start + body.node().range();

        DeclarationNode::raw(
            Declaration::Function {
                name,
                parameters: parameters.unwrap_or_default(),
                body_type,
                body,
            },
            range,
        )
    })
}
