use crate::ast;
use kore::{
    format::{SeparateEach, SuffixEach},
    str,
};
use std::fmt::{Display, Formatter};

impl<Import, Declaration> Display for ast::Module<Import, Declaration>
where
    Import: Display,
    Declaration: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "{imports}{spacer}{declarations}",
            imports = SuffixEach(";\n", &self.imports),
            spacer = if self.imports.is_empty() || self.declarations.is_empty() {
                ""
            } else {
                "\n"
            },
            declarations = SuffixEach("\n", &self.declarations)
        )
    }
}

impl Display for ast::Import {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "use {source}/{path}{alias}",
            source = match &self.source {
                ast::ImportSource::Root => str!("@"),
                ast::ImportSource::Local => str!("."),
                ast::ImportSource::Named(name) => name.clone(),
                ast::ImportSource::Scoped { scope, name } => format!("@{scope}/{name}"),
            },
            path = SeparateEach("/", &self.path),
            alias = Alias(self.alias.as_deref())
        )
    }
}

struct Alias<'a>(Option<&'a str>);

impl Display for Alias<'_> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if let Some(x) = &self.0 {
            write!(f, " as {x}")
        } else {
            Ok(())
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn empty() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::module(
                ast::Storage::public(str!("foo")),
                ast::shape::Module(ast::Module::new(vec![], vec![]))
            ))
            .to_string(),
            "module foo {}"
        );
    }

    #[test]
    fn imports() {
        assert_eq!(
            ast::shape::Module(ast::Module::new(
                vec![ast::shape::Import(ast::Import::new(
                    ast::ImportSource::Root,
                    vec![str!("bar"), str!("fizz")],
                    None
                ))],
                vec![]
            ))
            .to_string(),
            "use @/bar/fizz;
"
        );
    }

    #[test]
    fn declarations() {
        assert_eq!(
            ast::shape::Module(ast::Module::new(
                vec![],
                vec![ast::shape::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("bar")),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    ))
                ))]
            ))
            .to_string(),
            "type bar = nil;
"
        );
    }

    #[test]
    fn imports_and_declarations() {
        assert_eq!(
            ast::shape::Module(ast::Module::new(
                vec![ast::shape::Import(ast::Import::new(
                    ast::ImportSource::Root,
                    vec![str!("bar"), str!("fizz")],
                    None
                ))],
                vec![ast::shape::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("bar")),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    ))
                ))]
            ))
            .to_string(),
            "use @/bar/fizz;

type bar = nil;
"
        );
    }
}
