use crate::{Position, Range};
use combine::{
    attempt, many, optional, parser, parser::char as p, position, value, Parser, Stream,
};
use kore::invariant;

pub fn span<T, R, P>(parser: P) -> impl Parser<T, Output = (R, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = R>,
{
    attempt((position(), parser, position()))
        .map(|(start, x, end)| (x, Range::from(&start, &end.decrement())))
}

pub fn lexeme<T, R, P>(parser: P) -> impl Parser<T, Output = (R, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = R>,
{
    span(parser).skip(p::spaces())
}

pub fn terminated<T, R, P>(parser: P) -> impl Parser<T, Output = R>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = R>,
{
    parser.skip(optional(symbol(';')))
}

pub fn between<T, R1, R2, R3, P1, P2, P3>(
    open: P1,
    close: P2,
    parser: P3,
) -> impl Parser<T, Output = (R3, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P1: Parser<T, Output = (R1, Range)>,
    P2: Parser<T, Output = (R2, Range)>,
    P3: Parser<T, Output = R3>,
{
    (open, parser, close).map(|((_, start), x, (_, end))| (x, &start + &end))
}

pub fn folding<T, R1, R2, P1, P2, F>(lhs: P1, rhs: P2, fold: F) -> impl Parser<T, Output = R1>
where
    T: Stream<Token = char>,
    P1: Parser<T, Output = R1>,
    P2: Parser<T, Output = R2>,
    F: Fn(R1, R2) -> R1,
{
    lhs.and(many::<Vec<_>, _, _>(rhs))
        .map(move |(lhs, args)| args.into_iter().fold(lhs, &fold))
}

pub fn symbol<T>(c: char) -> impl Parser<T, Output = (char, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    lexeme(p::char(c))
}

pub fn sequence<T>(sequence: &'static str) -> impl Parser<T, Output = &'static str>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    parser! {
        pub fn recurse[T](chars: Vec<char>)(T) -> ()
        where
            [T: Stream<Token = char>]
        {
            match &chars[..] {
                [] | [_] => invariant!("glyph must be at least 2 characters long"),

                [lhs, rhs] => (p::char(*lhs), p::spaces(), p::char(*rhs))
                    .with(value(()))
                    .left(),

                [first, rest @ ..] => (p::char(*first), p::spaces(), recurse(rest.into()))
                    .with(value(()))
                    .right(),
            }
        }
    }

    recurse(sequence.chars().collect()).with(value(sequence))
}

pub fn glyph<T>(glyph: &'static str) -> impl Parser<T, Output = (&'static str, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    lexeme(sequence(glyph))
}

pub fn keyword<T>(keyword: &'static str) -> impl Parser<T, Output = (&'static str, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    lexeme(p::string(keyword))
}

pub fn identifier<T, P>(prefix: P) -> impl Parser<T, Output = (String, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = char>,
{
    lexeme(prefix.then(|first| {
        many::<Vec<_>, _, _>(p::alpha_num().or(p::char('_')))
            .map(move |rest| [vec![first], rest].concat().into_iter().collect())
    }))
}

pub fn standard_identifier<T>() -> impl Parser<T, Output = (String, Range)>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    identifier(p::alpha_num().or(p::char('_')))
}

#[cfg(test)]
mod tests {
    use crate::{
        matcher,
        test::mock::{mock, MockResult, MOCK_TOKEN},
        Range,
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
            (MockResult, Range((1, 1), (1, 8)))
        );

        let with_trailing_space = format!("{} ", MOCK_TOKEN);
        assert_eq!(
            parse(&with_trailing_space).unwrap().0,
            (MockResult, Range((1, 1), (1, 8)))
        );
    }

    #[test]
    fn terminated() {
        let parse = |s| matcher::terminated(mock()).easy_parse(Stream::new(s));

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_semicolon = format!("{};", MOCK_TOKEN);
        assert_eq!(parse(&with_semicolon).unwrap().0, MockResult);
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
            (MockResult, Range((1, 1), (1, 10)))
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
                Range((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn symbol() {
        let parse = |s| matcher::symbol('+').easy_parse(Stream::new(s));

        assert_eq!(parse("+").unwrap().0, ('+', Range((1, 1), (1, 1))));
        assert_eq!(parse("+ ").unwrap().0, ('+', Range((1, 1), (1, 1))));
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

        assert_eq!(parse("&&").unwrap().0, ("&&", Range((1, 1), (1, 2))));
        assert_eq!(parse("& & ").unwrap().0, ("&&", Range((1, 1), (1, 3))));
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

        assert_eq!(parse("foo").unwrap().0, ("foo", Range((1, 1), (1, 3))));
        assert_eq!(parse("foo ").unwrap().0, ("foo", Range((1, 1), (1, 3))));
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
            (String::from("$foo_"), Range((1, 1), (1, 5)))
        );
        assert_eq!(
            parse("$").unwrap().0,
            (String::from("$"), Range((1, 1), (1, 1)))
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
            (String::from("foo"), Range((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("_foo").unwrap().0,
            (String::from("_foo"), Range((1, 1), (1, 4)))
        );
    }
}
