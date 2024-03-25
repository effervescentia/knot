use crate::{
    ast,
    matcher::{self as m, Position},
};
use combine::{position, value, Parser, Stream};
use lang::Range;

pub fn storage<T>(keyword: &'static str) -> impl Parser<T, Output = (ast::raw::Storage, Range)>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        position().map(|x: T::Position| x.to_point()),
        m::keyword("priv")
            .with(value(ast::Visibility::Private))
            .or(value(ast::Visibility::Public)),
        m::keyword(keyword),
        m::binding(),
    )
        .map(|(start, visibility, _, binding)| {
            let range = binding.0.range().extend(&start);

            (ast::Storage::new(visibility, binding), range)
        })
}
