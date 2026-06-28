use crate::matcher as m;
use combine::{choice, Parser, Stream};
use lang::ast;

fn expression<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Statement>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::terminated(parser).map(|inner| {
        let range = *inner.0.range();

        ast::raw::Statement::raw(ast::Statement::Expression(inner), range)
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

        ast::raw::Statement::raw(ast::Statement::Variable(name, value), range)
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
    use crate::expression;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::Statement> {
        super::statement(expression::expression)
            .skip(eof())
            .easy_parse(Stream::new(s))
    }

    #[test]
    fn expression() {
        let ast = parse("nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn variable() {
        let ast = parse("let foo = nil;").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }
}
