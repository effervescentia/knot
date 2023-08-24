use crate::declaration::{self, Declaration};
use crate::import::{self, Import};
use combine::{choice, many, Parser, Stream};

#[derive(Debug, PartialEq)]
enum Entry {
    Import(Import),
    Declaration(Declaration),
}

#[derive(Debug, PartialEq)]
pub struct Module {
    pub imports: Vec<Import>,
    pub declarations: Vec<Declaration>,
}

impl Module {
    pub fn new(imports: Vec<Import>, declarations: Vec<Declaration>) -> Module {
        Module {
            imports,
            declarations,
        }
    }
}

pub fn module<T>() -> impl Parser<T, Output = Module>
where
    T: Stream<Token = char>,
{
    many::<Vec<_>, _, _>(choice((
        import::import().map(Entry::Import),
        declaration::declaration().map(Entry::Declaration),
    )))
    .map(|entries| {
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
            })
    })
}

#[cfg(test)]
mod tests {
    use crate::{
        declaration::{Declaration, Storage, Visibility},
        expression::Expression,
        import::{self, Import},
        module::{self, Module},
        primitive::Primitive,
    };
    use combine::Parser;

    #[test]
    fn module() {
        let parse = |s| module::module().parse(s);

        assert_eq!(parse("").unwrap().0, Module::new(vec![], vec![]));
        assert_eq!(
            parse("use @/foo;").unwrap().0,
            Module::new(
                vec![Import::new(
                    import::Source::Root,
                    vec![String::from("foo")],
                    None
                )],
                vec![]
            )
        );
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            Module::new(
                vec![],
                vec![Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value_type: None,
                    value: Expression::Primitive(Primitive::Nil)
                }]
            )
        );
    }
}
