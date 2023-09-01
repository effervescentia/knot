use crate::range::Range;
use combine::{
    attempt, many, optional, parser, parser::char as p, position, value, Parser, Stream,
};
use std::fmt::Debug;

pub fn lexeme<T, R, P>(parser: P) -> impl Parser<T, Output = (R, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = R>,
{
    (position(), parser, position())
        .skip(p::spaces())
        .map(|(start, x, end)| (x, Range(start, end)))
}

pub fn terminated<T, R, P>(parser: P) -> impl Parser<T, Output = R>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = R>,
{
    parser.skip(optional(symbol(';')))
}

pub fn between<T, R1, R2, R3, P1, P2, P3>(
    open: P1,
    close: P2,
    parser: P3,
) -> impl Parser<T, Output = (R3, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P1: Parser<T, Output = (R1, Range<T>)>,
    P2: Parser<T, Output = (R2, Range<T>)>,
    P3: Parser<T, Output = R3>,
{
    (open, parser, close).map(|((_, start), x, (_, end))| (x, start.concat(&end)))
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

pub fn symbol<T>(c: char) -> impl Parser<T, Output = (char, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    lexeme(p::char(c))
}

pub fn glyph<T>(glyph: &'static str) -> impl Parser<T, Output = (&'static str, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    parser! {
        pub fn recurse[T](chars: Vec<char>)(T) -> ()
        where
            [T: Stream<Token = char>]
        {
            match &chars[..] {
                [] | [_] => panic!("glyph must be at least 2 characters long"),

                [lhs, rhs] => (p::char(*lhs), p::spaces(), p::char(*rhs))
                    .with(value(()))
                    .left(),

                [first, rest @ ..] => (p::char(*first), p::spaces(), recurse(rest.into()))
                    .with(value(()))
                    .right(),
            }
        }
    }

    lexeme(recurse(glyph.chars().collect::<Vec<char>>()).with(value(glyph)))
}

pub fn keyword<T>(keyword: &'static str) -> impl Parser<T, Output = (&'static str, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    attempt(lexeme(p::string(keyword)))
}

pub fn identifier<T>(
    prefix: impl Parser<T, Output = char>,
) -> impl Parser<T, Output = (String, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    lexeme(prefix.then(|first| {
        many::<Vec<_>, _, _>(p::alpha_num().or(p::char('_'))).map(move |mut rest| {
            let mut result = vec![first];
            result.append(&mut rest);
            result.into_iter().collect::<String>()
        })
    }))
}

pub fn standard_identifier<T>() -> impl Parser<T, Output = (String, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    identifier(p::alpha_num().or(p::char('_')))
}

#[cfg(test)]
mod tests {
    use crate::{
        matcher,
        mock::{mock, MockResult, MOCK_TOKEN},
        range::Range,
    };
    use combine::{error::StringStreamError, parser, Parser};

    #[test]
    fn lexeme() {
        let parse = |s| matcher::lexeme(mock()).parse(s);

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, (MockResult, Range::str(1, 1)));

        let with_trailing_space = format!("{} ", MOCK_TOKEN);
        assert_eq!(
            parse(with_trailing_space.as_str()).unwrap().0,
            (MockResult, Range::str(1, 1))
        );
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

        assert_eq!(parse("+").unwrap().0, ('+', Range::str(1, 1)));
        assert_eq!(parse("+ ").unwrap().0, ('+', Range::str(1, 1)));
        assert_eq!(parse("-"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn glyph() {
        let parse = |s| matcher::glyph("&&").parse(s);

        assert_eq!(parse("&&").unwrap().0, ("&&", Range::str(1, 1)));
        assert_eq!(parse("& & ").unwrap().0, ("&&", Range::str(1, 1)));
        assert_eq!(parse("||"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn keyword() {
        let parse = |s| matcher::keyword("foo").parse(s);

        assert_eq!(parse("foo").unwrap().0, ("foo", Range::str(1, 1)));
        assert_eq!(parse("foo ").unwrap().0, ("foo", Range::str(1, 1)));
        assert_eq!(parse("bar"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn identifier() {
        let parse = |s| matcher::identifier(parser::char::char('$')).parse(s);

        assert_eq!(
            parse("$foo_").unwrap().0,
            (String::from("$foo_"), Range::str(1, 1))
        );
        assert_eq!(parse("$").unwrap().0, (String::from("$"), Range::str(1, 1)));
        assert_eq!(parse(""), Err(StringStreamError::Eoi));
    }

    #[test]
    fn standard_identifier() {
        let parse = |s| matcher::standard_identifier().parse(s);

        assert_eq!(
            parse("foo").unwrap().0,
            (String::from("foo"), Range::str(1, 1))
        );
        assert_eq!(
            parse("_foo").unwrap().0,
            (String::from("_foo"), Range::str(1, 1))
        );
    }
}
