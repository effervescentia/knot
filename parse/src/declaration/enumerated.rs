use super::storage;
use crate::{matcher as m, types::type_expression};
use combine::{attempt, choice, optional, sep_end_by, Parser, Stream};
use lang::{
    ast::{Declaration, DeclarationNode, TypeExpressionNode},
    Position, Range,
};

fn variant<T>() -> impl Parser<T, Output = (String, Vec<TypeExpressionNode<()>>, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
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

pub fn enumerated<T>() -> impl Parser<T, Output = DeclarationNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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
