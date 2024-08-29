use super::storage;
use crate::{matcher as m, types::type_expression};
use combine::{attempt, choice, Parser, Stream};
use lang::ast;

pub fn variant<T>() -> impl Parser<T, Output = (String, Vec<ast::raw::TypeExpression>)>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        attempt((
            m::standard_identifier(),
            m::tuple(type_expression::type_expression()),
        ))
        .map(|((name, _), (parameters, _))| (name, parameters)),
        m::standard_identifier().map(|(name, _)| (name, vec![])),
    ))
}

pub fn enumerated<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("enum"),
        m::closure(m::surround_by(variant(), || m::symbol('|'))),
    ))
    .map(|((storage, start), (variants, end))| {
        let range = &start + &end;

        ast::raw::Declaration::raw(ast::Declaration::enumerated(storage, variants), range)
    })
}
