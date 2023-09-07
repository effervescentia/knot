use crate::{matcher as m, position::Decrement};
use combine::{choice, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Statement<E> {
    Effect(E),
    Variable(String, E),
}

fn effect<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = R>,
{
    m::terminated(parser).map(|inner| Statement::Effect(inner))
}

fn variable<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = R>,
{
    m::terminated((
        m::keyword("let"),
        m::standard_identifier(),
        m::symbol('='),
        parser,
    ))
    .map(|(_, (name, _), _, value)| Statement::Variable(name, value))
}

pub fn statement<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = R>,
{
    choice((effect(parser()), variable(parser())))
}

#[cfg(test)]
mod tests {
    use super::{statement, Statement};
    use crate::{
        test::mock::{mock, MockResult, MOCK_TOKEN},
        ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<Statement<MockResult>> {
        statement(mock).easy_parse(Stream::new(s))
    }

    #[test]
    fn effect() {
        let input = format!("{};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Effect(MockResult)
        );
    }

    #[test]
    fn variable() {
        let input = format!("let foo = {};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Variable(String::from("foo"), MockResult)
        );
    }
}
