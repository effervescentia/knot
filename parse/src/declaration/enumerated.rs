use super::storage;
use crate::{ast, matcher as m, types::type_expression};
use combine::{attempt, choice, optional, sep_end_by, sep_end_by1, Parser, Stream};
use kore::invariant;
use lang::Range;

fn variant<T>() -> impl Parser<T, Output = (String, Vec<ast::raw::TypeExpression>, Range)>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        attempt((
            m::standard_identifier(),
            m::between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by::<Vec<_>, _, _, _>(type_expression::type_expression(), m::symbol(',')),
            ),
        ))
        .map(|((name, start), (parameters, end))| (name, parameters, &start + &end)),
        m::standard_identifier().map(|(name, range)| (name, vec![], range)),
    ))
}

pub fn enumerated<T>() -> impl Parser<T, Output = ast::raw::Declaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        storage::storage("enum"),
        m::symbol('='),
        optional(m::symbol('|')).with(sep_end_by1::<Vec<_>, _, _, _>(variant(), m::symbol('|'))),
    ))
    .map(|((storage, start), _, variants)| {
        let end = &variants
            .last()
            .unwrap_or_else(|| {
                invariant!("enum should only parse successfully with at least one variant")
            })
            .2;
        let range = &start + end;

        ast::raw::Declaration::new(
            ast::Declaration::enumerated(
                storage,
                variants
                    .into_iter()
                    .map(|(name, parameters, _)| (name, parameters))
                    .collect(),
            ),
            range,
        )
    })
}
