use super::{storage, Declaration, DeclarationRaw};
use crate::{
    matcher as m,
    range::Range,
    types::type_expression::{self, TypeExpression},
};
use combine::{choice, many1, sep_end_by, Parser, Stream};
use std::fmt::Debug;

fn variant<T>() -> impl Parser<T, Output = (String, Vec<TypeExpression>, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::symbol('|').with(choice((
        (
            m::standard_identifier(),
            m::between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by::<Vec<_>, _, _, _>(type_expression::type_expression(), m::symbol(',')),
            ),
        )
            .map(|((name, start), (parameters, end))| (name, parameters, start.concat(&end))),
        m::standard_identifier().map(|(name, range)| (name, vec![], range)),
    )))
}

pub fn enumerated<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::terminated((
        storage::storage("enum"),
        m::symbol('='),
        many1::<Vec<_>, _, _>(variant()),
    ))
    .map(|((name, start), _, variants)| {
        let end = &variants.last().unwrap().2;
        let range = start.concat(end);
        DeclarationRaw(
            Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, parameters, _)| (name, parameters))
                    .collect(),
            },
            range,
        )
    })
}
