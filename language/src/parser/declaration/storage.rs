use crate::{
    ast::storage::{Storage, Visibility},
    common::{position::Decrement, range::Range},
    parser::matcher as m,
};
use combine::{position, value, Parser, Stream};
use std::fmt::Debug;

pub fn storage<T>(keyword: &'static str) -> impl Parser<T, Output = (Storage, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        position(),
        m::keyword("priv")
            .with(value(Visibility::Private))
            .or(value(Visibility::Public)),
        m::keyword(keyword),
        m::standard_identifier(),
    )
        .map(|(start, visibility, _, (name, end))| (Storage(visibility, name), end.include(start)))
}
