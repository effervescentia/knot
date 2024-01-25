use crate::{ast, matcher as m};
use combine::{attempt, sep_end_by, Parser, Stream};

fn style_literal<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    let style_rule = || {
        (m::standard_identifier(), m::symbol(':'), parser()).map(|((lhs, _), _, rhs)| (lhs, rhs))
    };

    m::between(
        m::symbol('{'),
        m::symbol('}'),
        sep_end_by(style_rule(), m::symbol(',')),
    )
    .map(|(xs, range)| ast::raw::Expression::new(ast::Expression::Style(xs), range))
}

pub fn style<T, P, F>(parser: F) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
    F: Fn() -> P,
{
    attempt((m::keyword("style"), style_literal(parser))).map(
        |((_, start), ast::raw::Expression(node))| {
            ast::raw::Expression(node.map_range(|end| &start + &end))
        },
    )
}
