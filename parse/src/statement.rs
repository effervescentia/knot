use crate::matcher as m;
use combine::{choice, Parser, Stream};
use lang::{
    ast::{ExpressionNode, Statement, StatementNode},
    Position,
};

fn expression<T, P>(parser: P) -> impl Parser<T, Output = StatementNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    m::terminated(parser).map(|inner| {
        let range = inner.node().range().clone();

        StatementNode::raw(Statement::Expression(inner), range)
    })
}

fn variable<T, P>(parser: P) -> impl Parser<T, Output = StatementNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
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

pub fn statement<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = StatementNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    choice((variable(parser()), expression(parser())))
}

#[cfg(test)]
mod tests {
    use crate::expression;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use lang::{
        ast::{Expression, Primitive, Statement, StatementNode},
        test::fixture as f,
    };

    fn parse(s: &str) -> crate::Result<StatementNode<()>> {
        super::statement(expression::expression)
            .skip(eof())
            .easy_parse(Stream::new(s))
    }

    #[test]
    fn expression() {
        assert_eq!(
            parse("nil;").unwrap().0,
            f::n::sr(
                Statement::Expression(f::n::xr(
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
