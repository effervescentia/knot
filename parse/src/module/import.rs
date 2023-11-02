use crate::{matcher as m, Position, Range};
use combine::{
    between, choice, many1, not_followed_by, optional, parser::char as p, sep_end_by, value,
    Parser, Stream,
};
use kore::invariant;
use lang::ast::{AstNode, Import, ImportNode, ImportSource, ImportSourceNode, ImportTarget};

// use @/x;
// use @/x.{a, b};
// use ./y;
// use ./y.{c, d};
// use external/z;
// use external/z.{e, f};
// use @scope/external/z;
// use @scope/external/z.{e, f};

pub fn import<T>() -> impl Parser<T, Output = ImportNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    let source = || {
        choice((
            m::symbol('@')
                .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
                .map(|(_, range)| (ImportSource::Root, range)),
            m::symbol('.').map(|(_, range)| (ImportSource::Local, range)),
            m::standard_identifier().map(|(x, range)| (ImportSource::Named(x), range)),
            (
                m::identifier(p::char('@')),
                p::char('/'),
                m::standard_identifier(),
            )
                .map(|((scope, start), _, (name, end))| {
                    (
                        ImportSource::Scoped {
                            scope: scope
                                .strip_prefix('@')
                                .unwrap_or_else(|| {
                                    invariant!("scope should always begin with '@' character")
                                })
                                .to_owned(),
                            name,
                        },
                        &start + &end,
                    )
                }),
        ))
        .map(|(x, range)| ImportSourceNode::<Range, ()>::raw(x, range))
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
    .map(|((_, start), source, path, aliases)| {
        ImportNode::<Range, ()>::raw(
            Import {
                source,
                path,
                aliases,
            },
            start,
        )
    })
}

#[cfg(test)]
mod tests {
    use super::{Import, ImportSource, ImportTarget};
    use crate::{test::fixture as f, Range};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use lang::ast::ImportNode;

    fn parse(s: &str) -> crate::Result<ImportNode<Range, ()>> {
        super::import().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::n::ir(
                Import::new(
                    f::n::i::sr(ImportSource::Root, ((1, 1), (1, 1))),
                    vec![String::from("foo")],
                    None
                ),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_nested() {
        assert_eq!(
            parse("use @/foo/bar/fizz;").unwrap().0,
            f::n::ir(
                Import::new(
                    f::n::i::sr(ImportSource::Root, ((1, 1), (1, 1))),
                    vec![
                        String::from("foo"),
                        String::from("bar"),
                        String::from("fizz")
                    ],
                    None
                ),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_named_empty() {
        assert_eq!(
            parse("use @/foo.{};").unwrap().0,
            f::n::ir(
                Import::new(
                    f::n::i::sr(ImportSource::Root, ((1, 1), (1, 1))),
                    vec![String::from("foo")],
                    Some(vec![])
                ),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_named() {
        assert_eq!(
            parse("use @/foo.{*, bar};").unwrap().0,
            f::n::ir(
                Import::new(
                    f::n::i::sr(ImportSource::Root, ((1, 1), (1, 1))),
                    vec![String::from("foo")],
                    Some(vec![
                        (ImportTarget::Module, None),
                        (ImportTarget::Named(String::from("bar")), None)
                    ])
                ),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo.{* as foo, fizz as buzz};").unwrap().0,
            f::n::ir(
                Import::new(
                    f::n::i::sr(ImportSource::Root, ((1, 1), (1, 1))),
                    vec![String::from("foo")],
                    Some(vec![
                        (ImportTarget::Module, Some(String::from("foo"))),
                        (
                            ImportTarget::Named(String::from("fizz")),
                            Some(String::from("buzz"))
                        )
                    ])
                ),
                ((1, 1), (1, 3))
            )
        );
    }
}
