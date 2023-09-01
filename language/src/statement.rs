use crate::matcher as m;
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
    T::Position: Copy + Debug,
    P: Parser<T, Output = R>,
{
    m::terminated(parser).map(|inner| Statement::Effect(inner))
}

fn variable<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
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
    T::Position: Copy + Debug,
    P: Parser<T, Output = R>,
{
    choice((effect(parser()), variable(parser())))
}

#[cfg(test)]
mod tests {
    use crate::{
        mock::{mock, MockResult, MOCK_TOKEN},
        statement::{statement, Statement},
    };
    use combine::Parser;

    #[test]
    fn statement_effect() {
        let parse = |s| statement(mock).parse(s);

        let input = format!("{};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Effect(MockResult)
        );
    }

    #[test]
    fn statement_variable() {
        let parse = |s| statement(mock).parse(s);

        let input = format!("let foo = {};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Variable(String::from("foo"), MockResult)
        );
    }
}
