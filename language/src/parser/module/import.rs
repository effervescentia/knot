use crate::{
    ast::import::{Import, Source, Target},
    common::position::Decrement,
    parser::matcher as m,
};
use combine::{
    between, choice, many1, not_followed_by, optional, parser::char as p, sep_end_by, value,
    Parser, Stream,
};
use std::fmt::Debug;

// use @/x;
// use @/x.{a, b};
// use ./y;
// use ./y.{c, d};
// use external/z;
// use external/z.{e, f};
// use @scope/external/z;
// use @scope/external/z.{e, f};

pub fn import<T>() -> impl Parser<T, Output = Import>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let source = || {
        choice((
            m::symbol('@')
                .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
                .with(value(Source::Root)),
            m::symbol('.').with(value(Source::Local)),
            choice((m::identifier(p::char('@')), m::standard_identifier()))
                .map(|(x, _)| Source::External(x)),
        ))
    };
    let path = || {
        m::symbol('/')
            .with(m::standard_identifier())
            .map(|(x, _)| x)
    };
    let alias = || {
        (
            choice((
                m::symbol('*').with(value(Target::Module)),
                m::standard_identifier().map(|(x, _)| Target::Named(x)),
            )),
            optional(
                m::keyword("as")
                    .with(m::standard_identifier())
                    .map(|(x, _)| x),
            ),
        )
    };

    m::terminated((
        m::keyword("use"),
        source(),
        many1(path()),
        optional(m::symbol('.').with(between(
            m::symbol('{'),
            m::symbol('}'),
            sep_end_by(alias(), m::symbol(',')),
        ))),
    ))
    .map(|(_, source, path, aliases)| Import {
        source,
        path,
        aliases,
    })
}

#[cfg(test)]
mod tests {
    use super::{Import, Source, Target};
    use crate::parser::ParseResult;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> ParseResult<Import> {
        super::import().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            Import::new(Source::Root, vec![String::from("foo")], None)
        );
    }

    #[test]
    fn import_nested() {
        assert_eq!(
            parse("use @/foo/bar/fizz;").unwrap().0,
            Import::new(
                Source::Root,
                vec![
                    String::from("foo"),
                    String::from("bar"),
                    String::from("fizz")
                ],
                None
            )
        );
    }

    #[test]
    fn import_named_empty() {
        assert_eq!(
            parse("use @/foo.{};").unwrap().0,
            Import::new(Source::Root, vec![String::from("foo")], Some(vec![]))
        );
    }

    #[test]
    fn import_named() {
        assert_eq!(
            parse("use @/foo.{*, bar};").unwrap().0,
            Import::new(
                Source::Root,
                vec![String::from("foo")],
                Some(vec![
                    (Target::Module, None),
                    (Target::Named(String::from("bar")), None)
                ])
            )
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo.{* as foo, fizz as buzz};").unwrap().0,
            Import::new(
                Source::Root,
                vec![String::from("foo")],
                Some(vec![
                    (Target::Module, Some(String::from("foo"))),
                    (
                        Target::Named(String::from("fizz")),
                        Some(String::from("buzz"))
                    )
                ])
            )
        );
    }
}
