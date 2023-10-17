use super::{Expression, ExpressionNode};
use crate::matcher as m;
use combine::{attempt, sep_end_by, Parser, Stream};
use knot_language::Position;

fn style_literal<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
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

pub fn style<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    attempt((m::keyword("style"), style_literal(parser))).map(
        |((_, start), ExpressionNode(node))| ExpressionNode(node.map_range(|end| &start + &end)),
    )
}
