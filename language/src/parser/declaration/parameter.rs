use crate::{
    ast::{Parameter, ParameterNode},
    common::position::Position,
    parser::{expression, matcher as m, types::typedef},
};
use combine::{optional, Parser, Stream};

pub fn parameter<T>() -> impl Parser<T, Output = ParameterNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    (
        m::standard_identifier(),
        typedef::typedef(),
        optional(m::symbol('=').with(expression::expression())),
    )
        .map(|((name, start), value_type, default_value)| {
            let mut range = start;

            if let Some(x) = &value_type {
                range = &range + x.node().range();
            }

            if let Some(x) = &default_value {
                range = &range + x.node().range();
            }

            ParameterNode::raw(Parameter::new(name, value_type, default_value), range)
        })
}
