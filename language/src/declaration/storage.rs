use crate::{matcher as m, range::Range};
use combine::{position, value, Parser, Stream};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Visibility {
    Public,
    Private,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Storage(pub Visibility, pub String);

pub fn storage<T>(keyword: &'static str) -> impl Parser<T, Output = (Storage, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
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
