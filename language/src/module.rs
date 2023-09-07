use crate::{
    declaration::{self, DeclarationRaw},
    import::{self, Import},
    position::Decrement,
};
use combine::{choice, many, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
enum Entry<D> {
    Import(Import),
    Declaration(D),
}

#[derive(Debug, PartialEq)]
pub struct Module<D> {
    pub imports: Vec<Import>,
    pub declarations: Vec<D>,
}

impl<D> Module<D> {
    pub fn new(imports: Vec<Import>, declarations: Vec<D>) -> Self {
        Self {
            imports,
            declarations,
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct ModuleRaw<T>(pub Module<DeclarationRaw<T>>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T> ModuleRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
}

pub fn module<T>() -> impl Parser<T, Output = ModuleRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    )))
    .map(|entries| {
        ModuleRaw(
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
        declaration::{
            storage::{Storage, Visibility},
            Declaration, DeclarationRaw,
        },
        expression::{primitive::Primitive, Expression, ExpressionRaw},
        import::{self, Import},
        module::{self, Module, ModuleRaw},
        range::Range,
        CharStream, ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<ModuleRaw<CharStream>> {
        module::module().easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(parse("").unwrap().0, ModuleRaw(Module::new(vec![], vec![])));
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            ModuleRaw(Module::new(
                vec![Import::new(
                    import::Source::Root,
                    vec![String::from("foo")],
                    None
                )],
                vec![]
            ))
        );
    }

    #[test]
    fn module_declaration() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            ModuleRaw(Module::new(
                vec![],
                vec![DeclarationRaw(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("foo")),
                        value_type: None,
                        value: ExpressionRaw(
                            Expression::Primitive(Primitive::Nil),
                            Range::chars((1, 13), (1, 15))
                        )
                    },
                    Range::chars((1, 1), (1, 15))
                )]
            ))
        );
    }
}
