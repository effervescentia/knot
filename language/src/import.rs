use crate::{matcher as m, position::Decrement};
use combine::{
    between, choice, many1, not_followed_by, optional, parser::char as p, sep_end_by, value,
    Parser, Stream,
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Source {
    Root,
    Local,
    External(String),
}

#[derive(Clone, Debug, PartialEq)]
pub enum Target {
    Named(String),
    Module,
}

#[derive(Debug, PartialEq)]
pub struct Import {
    pub source: Source,
    pub path: Vec<String>,
    pub aliases: Option<Vec<(Target, Option<String>)>>,
}

impl Import {
    pub fn new(
        source: Source,
        path: Vec<String>,
        aliases: Option<Vec<(Target, Option<String>)>>,
    ) -> Import {
        Import {
            source,
            path,
            aliases,
        }
    }
}

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
    use crate::{
        import::{self, Import, Source, Target},
        ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<Import> {
        import::import().easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: None
            }
        );
    }

    #[test]
    fn import_nested() {
        assert_eq!(
            parse("use @/foo/bar/fizz;").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![
                    String::from("foo"),
                    String::from("bar"),
                    String::from("fizz")
                ],
                aliases: None
            }
        );
    }

    #[test]
    fn import_named_empty() {
        assert_eq!(
            parse("use @/foo.{};").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: Some(vec![])
            }
        );
    }

    #[test]
    fn import_named() {
        assert_eq!(
            parse("use @/foo.{*, bar};").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: Some(vec![
                    (Target::Module, None),
                    (Target::Named(String::from("bar")), None)
                ])
            }
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo.{* as foo, fizz as buzz};").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: Some(vec![
                    (Target::Module, Some(String::from("foo"))),
                    (
                        Target::Named(String::from("fizz")),
                        Some(String::from("buzz"))
                    )
                ])
            }
        );
    }
}
