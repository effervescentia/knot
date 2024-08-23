pub mod import;

use crate::{declaration, matcher as m};
use combine::{choice, many, Parser, Stream};
use lang::ast;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
enum Entry<I, D> {
    Import(I),
    Declaration(D),
}

pub fn module<T>() -> impl Parser<T, Output = ast::raw::Module>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::span(many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    ))))
    .map(|(entries, range)| {
        ast::raw::Module::raw(
            entries
                .into_iter()
                .fold(ast::Module::new(vec![], vec![]), |mut acc, el| {
                    match el {
                        Entry::Import(import) => {
                            acc.imports.push(import);
                        }
                        Entry::Declaration(declaration) => {
                            acc.declarations.push(declaration);
                        }
                    }

                    acc
                }),
            range,
        )
    })
}

#[cfg(test)]
mod tests {

    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::{assert_eq, str};
    use lang::{ast, test::fixture, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::Module> {
        super::module().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("").unwrap().0,
            ast::raw::Module::raw(ast::Module::new(vec![], vec![]), Range::new((1, 1), (1, 1)))
        );
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            ast::raw::Module::raw(
                ast::Module::new(
                    vec![ast::raw::Import::raw(
                        ast::Import::new(ast::ImportSource::Root, vec![str!("foo")], None),
                        Range::new((1, 1), (1, 9))
                    )],
                    vec![]
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn module_declaration() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            ast::raw::Module::raw(
                ast::Module::new(
                    vec![],
                    vec![ast::raw::Declaration::raw(
                        ast::Declaration::constant(
                            ast::Storage::public(ast::raw::Binding::new(
                                ast::Binding(str!("foo")),
                                Range::new((1, 7), (1, 9))
                            )),
                            None,
                            ast::raw::Expression::raw(
                                ast::Expression::Primitive(ast::Primitive::Nil),
                                Range::new((1, 13), (1, 15))
                            )
                        ),
                        Range::new((1, 1), (1, 15))
                    )]
                ),
                Range::new((1, 1), (1, 16))
            )
        );
    }

    #[test]
    fn multiple_declarations() {
        let source = format!(
            "{import}

{type_alias}

{enumerated}

{constant}

{function}

{view}

{module}",
            import = fixture::import::SOURCE,
            type_alias = fixture::type_alias::SOURCE,
            enumerated = fixture::enumerated::SOURCE,
            constant = fixture::constant::SOURCE,
            function = fixture::function::SOURCE,
            view = fixture::view::SOURCE,
            module = fixture::module::SOURCE,
        );

        assert_eq!(
            parse(&source).unwrap().0,
            ast::raw::Module::raw(
                ast::Module::new(
                    vec![fixture::import::raw_at((0, 0))],
                    vec![
                        fixture::type_alias::raw_at((2, 0)),
                        fixture::enumerated::raw_at((4, 0)),
                        fixture::constant::raw_at((9, 0)),
                        fixture::function::raw_at((11, 0)),
                        fixture::view::raw_at((13, 0)),
                        fixture::module::raw_at((22, 0))
                    ]
                ),
                Range::new((1, 1), (30, 1))
            )
        );
    }
}
