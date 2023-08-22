use crate::combine::parser::char as p;
use crate::matcher as m;
use combine::{
    between, choice, many1, not_followed_by, optional, sep_end_by, value, Parser, Stream,
};

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
    source: Source,
    path: Vec<String>,
    aliases: Option<Vec<(Target, Option<String>)>>,
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
{
    let source = || {
        choice((
            m::symbol('@')
                .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
                .with(value(Source::Root)),
            m::symbol('.').with(value(Source::Local)),
            choice((m::identifier(p::char('@')), m::standard_identifier())).map(Source::External),
        ))
    };
    let path = || m::symbol('/').with(m::standard_identifier());
    let alias = || {
        (
            choice((
                m::symbol('*').with(value(Target::Module)),
                m::standard_identifier().map(Target::Named),
            )),
            optional(m::keyword("as").with(m::standard_identifier())),
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
    use crate::import::{self, Import, Source, Target};
    use combine::Parser;

    #[test]
    fn import() {
        let parse = |s| import::import().parse(s);

        assert_eq!(
            parse("use @/foo;").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: None
            }
        );
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
        assert_eq!(
            parse("use @/foo.{};").unwrap().0,
            Import {
                source: Source::Root,
                path: vec![String::from("foo")],
                aliases: Some(vec![])
            }
        );
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
