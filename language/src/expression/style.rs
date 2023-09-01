use super::{Expression, ExpressionRaw};
use crate::matcher as m;
use combine::{attempt, sep_end_by, Parser, Stream};
use std::fmt::Debug;

fn style_literal<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    let style_rule = || {
        (m::standard_identifier(), m::symbol(':'), parser()).map(|((lhs, _), _, rhs)| (lhs, rhs))
    };

    m::between(
        m::symbol('{'),
        m::symbol('}'),
        sep_end_by(style_rule(), m::symbol(',')),
    )
    .map(|(xs, range)| ExpressionRaw(Expression::Style(xs), range))
}

pub fn style<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    attempt(m::keyword("style").with(style_literal(parser)))
}
