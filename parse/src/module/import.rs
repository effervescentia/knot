use crate::{matcher as m, Position};
use combine::{
    between, choice, many1, not_followed_by, optional, parser::char as p, sep_end_by, value,
    Parser, Stream,
};
use lang::ast::{Import, ImportSource, ImportTarget};

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
    T::Position: Position,
{
    let source = || {
        choice((
            m::symbol('@')
                .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
                .with(value(ImportSource::Root)),
            m::symbol('.').with(value(ImportSource::Local)),
            choice((m::identifier(p::char('@')), m::standard_identifier()))
                .map(|(x, _)| ImportSource::External(x)),
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
                m::symbol('*').with(value(ImportTarget::Module)),
                m::standard_identifier().map(|(x, _)| ImportTarget::Named(x)),
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
    use super::{Import, ImportSource, ImportTarget};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> crate::Result<Import> {
        super::import().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            Import::new(ImportSource::Root, vec![String::from("foo")], None)
        );
    }

    #[test]
    fn import_nested() {
        assert_eq!(
            parse("use @/foo/bar/fizz;").unwrap().0,
            Import::new(
                ImportSource::Root,
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
            Import::new(ImportSource::Root, vec![String::from("foo")], Some(vec![]))
        );
    }

    #[test]
    fn import_named() {
        assert_eq!(
            parse("use @/foo.{*, bar};").unwrap().0,
            Import::new(
                ImportSource::Root,
                vec![String::from("foo")],
                Some(vec![
                    (ImportTarget::Module, None),
                    (ImportTarget::Named(String::from("bar")), None)
                ])
            )
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo.{* as foo, fizz as buzz};").unwrap().0,
            Import::new(
                ImportSource::Root,
                vec![String::from("foo")],
                Some(vec![
                    (ImportTarget::Module, Some(String::from("foo"))),
                    (
                        ImportTarget::Named(String::from("fizz")),
                        Some(String::from("buzz"))
                    )
                ])
            )
        );
    }
}
