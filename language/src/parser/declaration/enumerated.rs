use super::{storage, Declaration, DeclarationNode};
use crate::parser::{
    matcher as m,
    position::Decrement,
    range::Range,
    types::type_expression::{self, TypeExpressionRaw},
};
use combine::{attempt, choice, optional, sep_end_by, Parser, Stream};
use std::fmt::Debug;

fn variant<T>() -> impl Parser<T, Output = (String, Vec<TypeExpressionRaw<T>>, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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

pub fn enumerated<T>() -> impl Parser<T, Output = DeclarationNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::terminated((
        storage::storage("enum"),
        m::symbol('='),
        optional(m::symbol('|')).with(sep_end_by::<Vec<_>, _, _, _>(variant(), m::symbol('|'))),
    ))
    .map(|((name, start), _, variants)| {
        let end = &variants.last().unwrap().2;
        let range = &start + &end;

        DeclarationNode::raw(
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
