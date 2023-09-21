use crate::{
    ast::{
        expression::ExpressionNode,
        statement::{Statement, StatementNode},
    },
    common::position::Decrement,
    parser::matcher as m,
};
use combine::{choice, Parser, Stream};
use std::fmt::Debug;

fn effect<T, P>(parser: P) -> impl Parser<T, Output = StatementNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    m::terminated(parser).map(|inner| {
        let range = inner.node().range().clone();

        StatementNode::raw(Statement::Effect(inner), range)
    })
}

fn variable<T, P>(parser: P) -> impl Parser<T, Output = StatementNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    m::terminated((
        m::keyword("let"),
        m::standard_identifier(),
        m::symbol('='),
        parser,
    ))
    .map(|((_, start), (name, _), _, value)| {
        let end = value.node().range().clone();
        let range = &start + &end;

        StatementNode::raw(Statement::Variable(name, value), range)
    })
}

pub fn statement<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = StatementNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    choice((variable(parser()), effect(parser())))
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            expression::{Expression, Primitive},
            statement::{Statement, StatementNode},
        },
        parser::{expression, CharStream, ParseResult},
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<StatementNode<CharStream, ()>> {
        super::statement(expression::expression).easy_parse(Stream::new(s))
    }

    #[test]
    fn effect() {
        assert_eq!(
            parse("nil;").unwrap().0,
            f::n::sr(
                Statement::Effect(f::n::xr(
                    Expression::Primitive(Primitive::Nil),
                    ((1, 1), (1, 3))
                )),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            parse("let foo = nil;").unwrap().0,
            f::n::sr(
                Statement::Variable(
                    String::from("foo"),
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 11), (1, 13)))
                ),
                ((1, 1), (1, 13))
            )
        );
    }
}
