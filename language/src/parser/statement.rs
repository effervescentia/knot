use crate::parser::{
    expression::ExpressionNode, matcher as m, node::Node, position::Decrement, range::Range,
};
use combine::{choice, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Statement<E> {
    Effect(E),
    Variable(String, E),
}

pub type NodeValue<T, C> = Statement<ExpressionNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct StatementNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> StatementNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<NodeValue<T, C>, T, C> {
        &self.0
    }
}

impl<T> StatementNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}

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
    use super::{statement, Statement, StatementNode};
    use crate::{
        parser::{
            expression::{self, primitive::Primitive, Expression},
            CharStream, ParseResult,
        },
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<StatementNode<CharStream, ()>> {
        statement(expression::expression).easy_parse(Stream::new(s))
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
