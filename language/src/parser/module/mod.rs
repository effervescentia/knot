pub mod import;

use crate::{
    ast::{
        import::Import,
        module::{Module, ModuleNode},
    },
    common::position::Decrement,
    parser::declaration,
};
use combine::{choice, many, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
enum Entry<D> {
    Import(Import),
    Declaration(D),
}

pub fn module<T>() -> impl Parser<T, Output = ModuleNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    )))
    .map(|entries| {
        ModuleNode::raw(
            entries
                .into_iter()
                .fold(Module::new(vec![], vec![]), |mut acc, el| {
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
        )
    })
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            expression::{Expression, Primitive},
            import::{Import, Source},
            module::{Module, ModuleNode},
        },
        parser::{CharStream, ParseResult},
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<ModuleNode<CharStream, ()>> {
        super::module().easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(parse("").unwrap().0, f::n::mr(Module::new(vec![], vec![])));
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::n::mr(Module::new(
                vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                vec![]
            ))
        );
    }

    #[test]
    fn module_declaration() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            f::n::mr(Module::new(
                vec![],
                vec![f::n::dr(
                    f::a::const_(
                        "foo",
                        None,
                        f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                    ),
                    ((1, 1), (1, 15))
                )]
            ))
        );
    }
}
