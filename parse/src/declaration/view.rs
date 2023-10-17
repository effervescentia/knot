use super::{parameter, storage};
use crate::{expression, matcher as m};
use combine::{between, optional, sep_end_by, Parser, Stream};
use lang::{
    ast::{Declaration, DeclarationNode},
    Position,
};

// view foo -> nil;
// view foo -> {};
// view foo -> { nil; };
// view foo: nil -> nil;
// view foo() -> nil;
// view foo(): nil -> nil;
// view foo(props) -> nil;
// view foo({a, b: nil, c = 123}) -> nil;

pub fn view<T>() -> impl Parser<T, Output = DeclarationNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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
        let range = &start + body.node().range();
        DeclarationNode::raw(
            Declaration::View {
                name,
                parameters: attributes.unwrap_or(vec![]),
                body,
            },
            range,
        )
    })
}
