use crate::matcher as m;
use combine::{choice, many1, not_followed_by, optional, parser::char as p, value, Parser, Stream};
use lang::{ast, Range};

// use @/x;
// use @/x as xx;
// use ./y;
// use ./y as yy;
// use external/z;
// use external/z as zz;
// use @scope/external/z;
// use @scope/external/z as zz;

fn import_source<T>() -> impl Parser<T, Output = ast::ImportSource>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        m::symbol('@')
            .skip(not_followed_by(p::alpha_num().or(p::char('_'))))
            .with(value(ast::ImportSource::Root)),
        m::symbol('.').with(value(ast::ImportSource::Local)),
        choice((m::identifier(p::char('@')), m::standard_identifier()))
            .map(|(x, _)| ast::ImportSource::Named(x)),
    ))
}

fn import_path<T>() -> impl Parser<T, Output = (Vec<String>, Range)>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    many1(m::symbol('/').with(m::standard_identifier())).map(|xs: Vec<_>| {
        let (paths, ranges): (Vec<_>, Vec<_>) = xs.into_iter().unzip();
        // `unwrap()` is safe here because we are using `many1()`
        let range = ranges.into_iter().reduce(|acc, x| &acc + &x).unwrap();

        (paths, range)
    })
}

fn import_alias<T>() -> impl Parser<T, Output = Option<(String, Range)>>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    optional(m::keyword("as").with(m::standard_identifier()))
}

pub fn import<T>() -> impl Parser<T, Output = ast::raw::Import>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("use"),
        import_source(),
        optional(import_path()),
        import_alias(),
    ))
    .map(|((_, start), source, path, alias)| {
        let (alias, path, end_range) = match (alias, path) {
            (Some((alias, range)), path) => {
                (Some(alias), path.map(|x| x.0).unwrap_or_default(), range)
            }
            (None, Some((path, range))) => (None, path, range),
            (None, None) => (None, vec![], start),
        };

        let range = &start + &end_range;

        ast::raw::Import::raw(
            ast::Import {
                source,
                path,
                alias,
            },
            range,
        )
    })
}

#[cfg(test)]
mod tests {
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::{assert_eq, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::Import> {
        super::import().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            ast::raw::Import::raw(
                ast::Import::new(ast::ImportSource::Root, vec![str!("foo")], None),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn import_nested() {
        assert_eq!(
            parse("use @/foo/bar/fizz;").unwrap().0,
            ast::raw::Import::raw(
                ast::Import::new(
                    ast::ImportSource::Root,
                    vec![str!("foo"), str!("bar"), str!("fizz")],
                    None
                ),
                Range::new((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn import_named_no_alias() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            ast::raw::Import::raw(
                ast::Import::new(ast::ImportSource::Root, vec![str!("foo")], None),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn import_named_with_alias() {
        assert_eq!(
            parse("use @/foo/fizz as buzz;").unwrap().0,
            ast::raw::Import::raw(
                ast::Import::new(
                    ast::ImportSource::Root,
                    vec![str!("foo"), str!("fizz")],
                    Some(str!("buzz"))
                ),
                Range::new((1, 1), (1, 22))
            )
        );
    }
}
