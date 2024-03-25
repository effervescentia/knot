use crate::{ast, matcher as m};
use combine::{choice, Parser, Stream};

fn expression<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Statement>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::terminated(parser).map(|inner| {
        let range = *inner.0.range();

        ast::raw::Statement::new(ast::Statement::Expression(inner), range)
    })
}

fn variable<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Statement>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::terminated((
        m::keyword("let"),
        m::standard_identifier(),
        m::symbol('='),
        parser,
    ))
    .map(|((_, start), (name, _), _, value)| {
        let end = value.0.range();
        let range = &start + end;

        ast::raw::Statement::new(ast::Statement::Variable(name, value), range)
    })
}

pub fn statement<T, P, F>(parser: F) -> impl Parser<T, Output = ast::raw::Statement>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
    F: Fn() -> P,
{
    choice((variable(parser()), expression(parser())))
}

#[cfg(test)]
mod tests {
    use crate::{ast, expression};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::Range;

    fn parse(s: &str) -> crate::Result<ast::raw::Statement> {
        super::statement(expression::expression)
            .skip(eof())
            .easy_parse(Stream::new(s))
    }

    #[test]
    fn expression() {
        assert_eq!(
            parse("nil;").unwrap().0,
            ast::raw::Statement::new(
                ast::Statement::Expression(ast::raw::Expression::new(
                    ast::Expression::Primitive(ast::Primitive::Nil),
                    Range::new((1, 1), (1, 3))
                )),
                Range::new((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            parse("let foo = nil;").unwrap().0,
            ast::raw::Statement::new(
                ast::Statement::Variable(
                    str!("foo"),
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 11), (1, 13))
                    )
                ),
                Range::new((1, 1), (1, 13))
            )
        );
    }
}
