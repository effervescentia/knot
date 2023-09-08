use crate::parser::{position::Decrement, range::Range};
use combine::{
    attempt, many, optional, parser, parser::char as p, position, value, Parser, Stream,
};
use std::fmt::Debug;

pub fn lexeme<T, R, P>(parser: P) -> impl Parser<T, Output = (R, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = R>,
{
    attempt((position(), parser, position()))
        .skip(p::spaces())
        .map(|(start, x, end)| (x, Range(start, end.decrement())))
}

pub fn terminated<T, R, P>(parser: P) -> impl Parser<T, Output = R>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
    P1: Parser<T, Output = (R1, Range<T>)>,
    P2: Parser<T, Output = (R2, Range<T>)>,
    P3: Parser<T, Output = R3>,
{
    (open, parser, close).map(|((_, start), x, (_, end))| (x, &start + &end))
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
    T::Position: Copy + Debug + Decrement,
{
    lexeme(p::char(c))
}

pub fn glyph<T>(glyph: &'static str) -> impl Parser<T, Output = (&'static str, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
{
    lexeme(p::string(keyword))
}

pub fn identifier<T>(
    prefix: impl Parser<T, Output = char>,
) -> impl Parser<T, Output = (String, Range<T>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
{
    identifier(p::alpha_num().or(p::char('_')))
}

#[cfg(test)]
mod tests {
    use crate::{
        parser::{matcher, range::Range},
        test::mock::{mock, MockResult, MOCK_TOKEN},
    };
    use combine::{
        easy::{Error, Errors, Info},
        parser,
        stream::position::{SourcePosition, Stream},
        EasyParser, Parser,
    };

    #[test]
    fn lexeme() {
        let parse = |s| matcher::lexeme(mock()).easy_parse(Stream::new(s));

        assert_eq!(
            parse(MOCK_TOKEN).unwrap().0,
            (MockResult, Range::chars((1, 1), (1, 8)))
        );

        let with_trailing_space = format!("{} ", MOCK_TOKEN);
        assert_eq!(
            parse(with_trailing_space.as_str()).unwrap().0,
            (MockResult, Range::chars((1, 1), (1, 8)))
        );
    }

    #[test]
    fn terminated() {
        let parse = |s| matcher::terminated(mock()).easy_parse(Stream::new(s));

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_semicolon = format!("{};", MOCK_TOKEN);
        assert_eq!(parse(with_semicolon.as_str()).unwrap().0, MockResult);
    }

    #[test]
    fn between() {
        let parse = |s| {
            matcher::between(matcher::symbol('>'), matcher::symbol('<'), mock())
                .easy_parse(Stream::new(s))
        };

        let input = format!(">{}<", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            (MockResult, Range::chars((1, 1), (1, 10)))
        );
    }

    #[test]
    fn folding() {
        #[derive(Debug, PartialEq)]
        enum Stack<T> {
            Empty,
            Next(T, Box<Stack<T>>),
        }

        impl<T> Stack<T> {
            fn new(x: T) -> Self {
                Self::Next(x, Box::new(Self::Empty))
            }

            fn push(self, x: T) -> Self {
                Self::Next(x, Box::new(self))
            }
        }

        let parse = |s| {
            matcher::folding(
                matcher::lexeme(mock()).map(|(x, range)| (Stack::new(x), range)),
                matcher::symbol(',').with(matcher::lexeme(mock())),
                |(lhs, start), (rhs, end)| (lhs.push(rhs), &start + &end),
            )
            .easy_parse(Stream::new(s))
        };

        let input = format!("{}, {}", MOCK_TOKEN, MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            (
                Stack::Next(
                    MockResult,
                    Box::new(Stack::Next(MockResult, Box::new(Stack::Empty)))
                ),
                Range::chars((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn symbol() {
        let parse = |s| matcher::symbol('+').easy_parse(Stream::new(s));

        assert_eq!(parse("+").unwrap().0, ('+', Range::chars((1, 1), (1, 1))));
        assert_eq!(parse("+ ").unwrap().0, ('+', Range::chars((1, 1), (1, 1))));
        assert_eq!(
            parse("-"),
            Err(Errors::from_errors(
                SourcePosition { line: 1, column: 1 },
                vec![
                    Error::Unexpected(Info::Token('-')),
                    Error::Expected(Info::Token('+'))
                ]
            ))
        );
    }

    #[test]
    fn glyph() {
        let parse = |s| matcher::glyph("&&").easy_parse(Stream::new(s));

        assert_eq!(parse("&&").unwrap().0, ("&&", Range::chars((1, 1), (1, 2))));
        assert_eq!(
            parse("& & ").unwrap().0,
            ("&&", Range::chars((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("||"),
            Err(Errors::from_errors(
                SourcePosition { line: 1, column: 1 },
                vec![
                    Error::Unexpected(Info::Token('|')),
                    Error::Expected(Info::Token('&'))
                ]
            ))
        );
    }

    #[test]
    fn keyword() {
        let parse = |s| matcher::keyword("foo").easy_parse(Stream::new(s));

        assert_eq!(
            parse("foo").unwrap().0,
            ("foo", Range::chars((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("foo ").unwrap().0,
            ("foo", Range::chars((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("bar"),
            Err(Errors::from_errors(
                SourcePosition { line: 1, column: 1 },
                vec![
                    Error::Unexpected(Info::Token('b')),
                    Error::Expected(Info::Static("foo"))
                ]
            ))
        );
    }

    #[test]
    fn identifier() {
        let parse = |s| matcher::identifier(parser::char::char('$')).easy_parse(Stream::new(s));

        assert_eq!(
            parse("$foo_").unwrap().0,
            (String::from("$foo_"), Range::chars((1, 1), (1, 5)))
        );
        assert_eq!(
            parse("$").unwrap().0,
            (String::from("$"), Range::chars((1, 1), (1, 1)))
        );
        assert_eq!(
            parse(""),
            Err(Errors::from_errors(
                SourcePosition { line: 1, column: 1 },
                vec![Error::end_of_input(), Error::Expected(Info::Token('$'))]
            ))
        );
    }

    #[test]
    fn standard_identifier() {
        let parse = |s| matcher::standard_identifier().easy_parse(Stream::new(s));

        assert_eq!(
            parse("foo").unwrap().0,
            (String::from("foo"), Range::chars((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("_foo").unwrap().0,
            (String::from("_foo"), Range::chars((1, 1), (1, 4)))
        );
    }
}
