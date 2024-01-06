use crate::{matcher as m, Position, Range};
use combine::{choice, many1, not_followed_by, optional, parser::char as p, value, Parser, Stream};
use lang::ast::{AstNode, Import, ImportNode, ImportSource};

// use @/x;
// use @/x as xx;
// use ./y;
// use ./y as yy;
// use external/z;
// use external/z as zz;
// use @scope/external/z;
// use @scope/external/z as zz;

fn import_source<T>() -> impl Parser<T, Output = ImportSource>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    choice((
        m::symbol('@')
            .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
            .with(value(ImportSource::Root)),
        m::symbol('.').with(value(ImportSource::Local)),
        choice((m::identifier(p::char('@')), m::standard_identifier()))
            .map(|(x, _)| ImportSource::Named(x)),
    ))
}

fn import_path<T>() -> impl Parser<T, Output = Vec<String>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    many1(
        m::symbol('/')
            .with(m::standard_identifier())
            .map(|(x, _)| x),
    )
}

fn import_alias<T>() -> impl Parser<T, Output = Option<String>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    optional(
        m::keyword("as")
            .with(m::standard_identifier())
            .map(|(x, _)| x),
    )
}

pub fn import<T>() -> impl Parser<T, Output = ImportNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::terminated((
        m::keyword("use"),
        import_source(),
        import_path(),
        import_alias(),
    ))
    .map(|((_, start), source, path, alias)| {
        ImportNode::<Range, ()>::raw(
            Import {
                source,
                path,
                alias,
            },
            start,
        )
    })
}

#[cfg(test)]
mod tests {
    use super::{Import, ImportSource};
    use crate::{test::fixture as f, Range};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::ast::ImportNode;

    fn parse(s: &str) -> crate::Result<ImportNode<Range, ()>> {
        super::import().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::n::ir(
                Import::new(ImportSource::Root, vec![str!("foo")], None),
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
                    ImportSource::Root,
                    vec![str!("foo"), str!("bar"), str!("fizz")],
                    None
                ),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_named_no_alias() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::n::ir(
                Import::new(ImportSource::Root, vec![str!("foo")], None),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo/fizz as buzz;").unwrap().0,
            f::n::ir(
                Import::new(
                    ImportSource::Root,
                    vec![str!("foo"), str!("fizz")],
                    Some(str!("buzz"))
                ),
                ((1, 1), (1, 3))
            )
        );
    }
}
