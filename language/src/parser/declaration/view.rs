use super::{parameter, storage, Declaration, DeclarationNode};
use crate::parser::{expression, matcher as m, position::Decrement, range::Ranged};
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

pub fn view<T>() -> impl Parser<T, Output = DeclarationNode<T, ()>>
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
        let range = &start + body.0.range();
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
