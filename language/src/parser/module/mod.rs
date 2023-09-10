pub mod import;
use crate::{
    parser::declaration::{self, DeclarationNode},
    parser::position::Decrement,
};
use combine::{choice, many, Parser, Stream};
use import::Import;
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
pub struct ModuleNode<T, C>(pub Module<DeclarationNode<T, C>>, pub C)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T> ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: Module<DeclarationNode<T, ()>>) -> Self {
        Self(x, ())
    }
}

impl<T> ModuleNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn id(self) -> i32 {
        self.1
    }

    pub fn to_ref(self) -> Module<i32> {
        Module {
            imports: self.0.imports,
            declarations: self
                .0
                .declarations
                .into_iter()
                .map(|x| x.0.id())
                .collect::<Vec<_>>(),
        }
    }
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
    use super::{
        import::{self, Import},
        Module, ModuleNode,
    };
    use crate::{
        parser::{
            declaration::{
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{primitive::Primitive, Expression},
            CharStream, ParseResult,
        },
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<ModuleNode<CharStream, ()>> {
        super::module().easy_parse(Stream::new(s))
    }

    #[test]
    fn module_empty() {
        assert_eq!(parse("").unwrap().0, f::mr(Module::new(vec![], vec![])));
    }

    #[test]
    fn module_import() {
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            f::mr(Module::new(
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
            f::mr(Module::new(
                vec![],
                vec![f::dr(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("foo")),
                        value_type: None,
                        value: f::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                    },
                    ((1, 1), (1, 15))
                )]
            ))
        );
    }
}
