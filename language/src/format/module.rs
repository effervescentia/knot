use crate::ast::{AstNode, ImportNode, ImportSource, ModuleNode};
use kore::{
    format::{SeparateEach, TerminateEach},
    str,
};
use std::fmt::{Display, Formatter};

impl<R, C> Display for ModuleNode<R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "{imports}{spacer}{declarations}",
            imports = TerminateEach(";\n", &self.0.imports),
            spacer = if self.0.is_empty() { "" } else { "\n" },
            declarations = TerminateEach("\n", &self.0.declarations)
        )
    }
}

impl<R, C> Display for ImportNode<R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "use {source}/{path}",
            source = match &self.node().value().source {
                ImportSource::Root => str!("@"),
                ImportSource::Local => str!("."),
                ImportSource::Named(name) => name.clone(),
                ImportSource::Scoped { scope, name } => format!("@{scope}/{name}"),
            },
            path = SeparateEach("/", &self.node().value().path)
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{Import, ImportSource, Module, TypeExpression},
        test::fixture as f,
    };
    use kore::str;

    #[test]
    fn empty() {
        assert_eq!(
            f::n::d(f::a::module("foo", f::n::m(Module::new(vec![], vec![])))).to_string(),
            "module foo {}"
        );
    }

    #[test]
    fn imports() {
        assert_eq!(
            f::n::m(Module::new(
                vec![f::n::i(Import::new(
                    ImportSource::Root,
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
            f::n::m(Module::new(
                vec![],
                vec![f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Nil)))]
            ))
            .to_string(),
            "type bar = nil;
"
        );
    }

    #[test]
    fn imports_and_declarations() {
        assert_eq!(
            f::n::m(Module::new(
                vec![f::n::i(Import::new(
                    ImportSource::Root,
                    vec![str!("bar"), str!("fizz")],
                    None
                ))],
                vec![f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Nil)))]
            ))
            .to_string(),
            "use @/bar/fizz;

type bar = nil;
"
        );
    }
}
