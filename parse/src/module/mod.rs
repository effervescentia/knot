pub mod import;

use crate::{ast, declaration, matcher as m};
use combine::{choice, many, Parser, Stream};
use lang::Range;
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
    many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    )))
    .map(|entries| {
        ast::raw::Module::new(
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
            Range::new((0, 0), (0, 0)),
        )
    })
}

#[cfg(test)]
mod tests {
    use crate::{ast, test::fixture};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::{
        ast::{Module, Primitive},
        Range,
    };

    fn parse(s: &str) -> crate::Result<ast::raw::Module> {
        super::module().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("").unwrap().0,
            ast::raw::Module::new(ast::Module::new(vec![], vec![]), Range::new((1, 1), (1, 1)))
        );
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            ast::raw::Module::new(
                ast::Module::new(
                    vec![ast::raw::Import::new(
                        ast::Import::new(ast::ImportSource::Root, vec![str!("foo")], None),
                        Range::new((1, 1), (1, 3))
                    )],
                    vec![]
                ),
                Range::new((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn module_declaration() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            ast::raw::Module::new(
                Module::new(
                    vec![],
                    vec![ast::raw::Declaration::new(
                        ast::Declaration::constant(
                            ast::Storage::public(ast::raw::Binding::new(
                                ast::Binding(str!("foo")),
                                Range::new((0, 0), (0, 0))
                            )),
                            None,
                            ast::raw::Expression::new(
                                ast::Expression::Primitive(Primitive::Nil),
                                Range::new((1, 13), (1, 15))
                            )
                        ),
                        Range::new((1, 1), (1, 15))
                    )]
                ),
                Range::new((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn multiple_declarations() {
        let source = format!(
            "{type_alias}

{enumerated}

{constant}

{function}

{view}

{module}",
            type_alias = fixture::type_alias::SOURCE,
            enumerated = fixture::enumerated::SOURCE,
            constant = fixture::constant::SOURCE,
            function = fixture::function::SOURCE,
            view = fixture::view::SOURCE,
            module = fixture::module::SOURCE,
        );

        assert_eq!(
            parse(&source).unwrap().0,
            ast::raw::Module::new(
                Module::new(
                    vec![],
                    vec![
                        fixture::type_alias::raw((0, 0)),
                        fixture::enumerated::raw((2, 0)),
                        fixture::constant::raw((6, 0)),
                        fixture::function::raw((8, 0)),
                        fixture::view::raw((10, 0)),
                        fixture::module::raw((19, 0))
                    ]
                ),
                Range::new((1, 1), (25, 1))
            )
        );
    }
}
