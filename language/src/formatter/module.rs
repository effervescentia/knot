use super::TerminateEach;
use crate::{
    ast::{
        import::{Import, Source},
        module::ModuleNode,
    },
    common::position::Decrement,
    formatter::SeparateEach,
};
use combine::Stream;
use std::fmt::{Debug, Display, Formatter};

impl<T, C> Display for ModuleNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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

impl Display for Import {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "use {source}/{path}",
            source = match &self.source {
                Source::Root => "@",
                Source::Local => ".",
                Source::External(x) => x,
            },
            path = SeparateEach("/", &self.path)
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            import::{Import, Source},
            module::Module,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn empty() {
        assert_eq!(
            f::n::d(f::a::module("foo", f::n::mr(Module::new(vec![], vec![])))).to_string(),
            "module foo {}"
        );
    }

    #[test]
    fn imports() {
        assert_eq!(
            f::n::mr(Module::new(
                vec![Import::new(
                    Source::Root,
                    vec![String::from("bar"), String::from("fizz")],
                    None
                )],
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
            f::n::mr(Module::new(
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
            f::n::mr(Module::new(
                vec![Import::new(
                    Source::Root,
                    vec![String::from("bar"), String::from("fizz")],
                    None
                )],
                vec![f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Nil)))]
            ))
            .to_string(),
            "use @/bar/fizz;

type bar = nil;
"
        );
    }
}
