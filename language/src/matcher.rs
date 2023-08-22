use combine::parser::char as p;
use combine::{attempt, many, optional, satisfy, unexpected_any, value, Parser, Stream};

pub fn lexeme<T, R, P>(parser: P) -> impl Parser<T, Output = R>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    parser.skip(p::spaces())
}

pub fn terminated<T, R, P>(parser: P) -> impl Parser<T, Output = R>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    parser.skip(optional(symbol(';')))
}

pub fn folding<T, R1, R2, P1, P2>(
    lhs: P1,
    rhs: P2,
    fold: impl Fn(R1, R2) -> R1,
) -> impl Parser<T, Output = R1>
where
    T: Stream<Token = char>,
    P1: Parser<T, Output = R1>,
    P2: Parser<T, Output = R2>,
{
    lhs.and(many::<Vec<_>, _, _>(rhs))
        .map(move |(lhs, args)| args.into_iter().fold(lhs, |acc, el| fold(acc, el)))
}

pub fn symbol<T>(c: char) -> impl Parser<T, Output = ()>
where
    T: Stream<Token = char>,
{
    lexeme(p::char(c)).with(value(()))
}

pub fn glyph<T>(glyph: &'static str) -> impl Parser<T, Output = ()>
where
    T: Stream<Token = char>,
{
    let mut chars = glyph.chars().into_iter();

    // TODO: try to replace with `take_while`
    attempt(
        combine::many::<Vec<_>, _, _>(lexeme(satisfy(move |c| Some(c) == chars.next()))).then(
            |r| {
                if r.len() == glyph.len() {
                    value(()).left()
                } else {
                    unexpected_any("complete glyph not parsed").right()
                }
            },
        ),
    )
}

pub fn keyword<T>(keyword: &'static str) -> impl Parser<T, Output = ()>
where
    T: Stream<Token = char>,
{
    lexeme(p::string(keyword)).with(value(()))
}

pub fn identifier<T>(prefix: impl Parser<T, Output = char>) -> impl Parser<T, Output = String>
where
    T: Stream<Token = char>,
{
    lexeme(prefix.then(|first| {
        many::<Vec<_>, _, _>(p::alpha_num().or(p::char('_'))).map(move |mut rest| {
            let mut result = vec![first];
            result.append(&mut rest);
            result.into_iter().collect::<String>()
        })
    }))
}

pub fn standard_identifier<T>() -> impl Parser<T, Output = String>
where
    T: Stream<Token = char>,
{
    identifier(p::alpha_num().or(p::char('_')))
}

#[cfg(test)]
mod tests {
    use crate::matcher;
    use combine::error::StringStreamError;
    use combine::{parser, Parser, Stream};

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
    fn lexeme() {
        let parse = |s| matcher::lexeme(mock()).parse(s);

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_trailing_space = format!("{} ", MOCK_TOKEN);
        assert_eq!(parse(with_trailing_space.as_str()).unwrap().0, MockResult);
    }

    #[test]
    fn terminated() {
        let parse = |s| matcher::terminated(mock()).parse(s);

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_semicolon = format!("{};", MOCK_TOKEN);
        assert_eq!(parse(with_semicolon.as_str()).unwrap().0, MockResult);
    }

    #[test]
    fn symbol() {
        let parse = |s| matcher::symbol('+').parse(s);

        assert_eq!(parse("+").unwrap().0, ());
        assert_eq!(parse("+ ").unwrap().0, ());
        assert_eq!(parse("-"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn glyph() {
        let parse = |s| matcher::glyph("&&").parse(s);

        assert_eq!(parse("&&").unwrap().0, ());
        assert_eq!(parse("& & ").unwrap().0, ());
        assert_eq!(parse("||"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn keyword() {
        let parse = |s| matcher::keyword("foo").parse(s);

        assert_eq!(parse("foo").unwrap().0, ());
        assert_eq!(parse("foo ").unwrap().0, ());
        assert_eq!(parse("bar"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn identifier() {
        let parse = |s| matcher::identifier(parser::char::char('$')).parse(s);

        assert_eq!(parse("$foo_").unwrap().0, String::from("$foo_"));
        assert_eq!(parse("$").unwrap().0, String::from("$"));
        assert_eq!(parse(""), Err(StringStreamError::Eoi));
    }

    #[test]
    fn standard_identifier() {
        let parse = |s| matcher::standard_identifier().parse(s);

        assert_eq!(parse("foo").unwrap().0, String::from("foo"));
        assert_eq!(parse("_foo").unwrap().0, String::from("_foo"));
    }
}
