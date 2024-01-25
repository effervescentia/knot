use crate::{ast, expression, matcher as m, types::typedef};
use combine::{optional, Parser, Stream};

pub fn parameter<T>() -> impl Parser<T, Output = ast::raw::Parameter>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        m::binding(),
        typedef::typedef(),
        optional(m::symbol('=').with(expression::expression())),
    )
        .map(|(binding, value_type, default_value)| {
            let mut range = *binding.0.range();

            if let Some(x) = &value_type {
                range = &range + x.0.range();
            }

            if let Some(x) = &default_value {
                range = &range + x.0.range();
            }

            ast::raw::Parameter::new(
                ast::Parameter::new(binding, value_type, default_value),
                range,
            )
        })
}
