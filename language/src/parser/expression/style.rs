use super::{Expression, ExpressionNode};
use crate::parser::{matcher as m, position::Decrement};
use combine::{attempt, sep_end_by, Parser, Stream};
use std::fmt::Debug;

fn style_literal<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    let style_rule = || {
        (m::standard_identifier(), m::symbol(':'), parser()).map(|((lhs, _), _, rhs)| (lhs, rhs))
    };

    m::between(
        m::symbol('{'),
        m::symbol('}'),
        sep_end_by(style_rule(), m::symbol(',')),
    )
    .map(|(xs, range)| ExpressionNode::raw(Expression::Style(xs), range))
}

pub fn style<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    attempt((m::keyword("style"), style_literal(parser)))
        .map(|((_, start), ExpressionNode(x, end, _))| ExpressionNode::raw(x, &start + &end))
}
