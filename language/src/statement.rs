use crate::matcher as m;
use combine::{choice, Parser, Stream};

#[derive(Debug, PartialEq)]
pub enum Statement<T> {
    Effect(T),
    Variable(String, T),
}

fn effect<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    m::terminated(parser).map(|inner| Statement::Effect(inner))
}

fn variable<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    m::terminated((
        m::keyword("let"),
        m::standard_identifier(),
        m::symbol('='),
        parser,
    ))
    .map(|(_, name, _, value)| Statement::Variable(name, value))
}

pub fn statement<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Statement<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    choice((effect(parser()), variable(parser())))
}

#[cfg(test)]
mod tests {
    use crate::matcher;
    use crate::statement::{self, Statement};
    use combine::{Parser, Stream};

    const MOCK_TOKEN: &str = "__mock__";

    #[derive(Debug, PartialEq)]
    struct MockResult;

    fn mock<T>() -> impl Parser<T, Output = MockResult>
    where
        T: Stream<Token = char>,
    {
        matcher::keyword(MOCK_TOKEN).map(|_| MockResult)
    }

    #[test]
    fn statement_effect() {
        let parse = |s| statement::statement(mock).parse(s);

        let input = format!("{};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Effect(MockResult)
        );
    }

    #[test]
    fn statement_variable() {
        let parse = |s| statement::statement(mock).parse(s);

        let input = format!("let foo = {};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Variable(String::from("foo"), MockResult)
        );
    }
}
