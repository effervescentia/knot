use crate::{
    ast::storage::{Storage, Visibility},
    common::{position::Position, range::Range},
    parser::matcher as m,
};
use combine::{position, value, Parser, Stream};

pub fn storage<T>(keyword: &'static str) -> impl Parser<T, Output = (Storage, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
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
