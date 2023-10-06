use super::{indented, Block, Indented, Parameters, Typedef};
use crate::{
    ast::{
        declaration::{Declaration, DeclarationNode},
        storage::{Storage, Visibility},
    },
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::{Debug, Display, Formatter, Write};

impl<T, C> Display for DeclarationNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self.node().value() {
            Declaration::TypeAlias { name, value } => {
                write!(f, "{binding} = {value};", binding = Binding("type", name))
            }

            Declaration::Enumerated { name, variants } => {
                write!(
                    f,
                    "{binding} ={variants};",
                    binding = Binding("enum", name),
                    variants = Variants(variants)
                )
            }

            Declaration::Constant {
                name,
                value_type,
                value,
            } => {
                write!(
                    f,
                    "{binding}{typedef} = {value};",
                    binding = Binding("const", name),
                    typedef = Typedef(value_type)
                )
            }

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => {
                write!(
                    f,
                    "{binding}{parameters}{typedef} -> {body};",
                    parameters = Parameters(parameters),
                    binding = Binding("func", name),
                    typedef = Typedef(body_type)
                )
            }

            Declaration::View {
                name,
                parameters,
                body,
            } => {
                write!(
                    f,
                    "{binding}{parameters} -> {body};",
                    binding = Binding("view", name),
                    parameters = Parameters(parameters)
                )
            }

            Declaration::Module { name, value } => {
                write!(
                    f,
                    "{binding} {{{module}}}",
                    binding = Binding("module", name),
                    module = Indented(Block(value))
                )
            }
        }
    }
}

struct Binding<'a>(&'a str, &'a Storage);

impl<'a> Display for Binding<'a> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let Self(keyword, Storage(visibility, name)) = self;

        write!(
            f,
            "{visibility}{keyword} {name}",
            visibility = if *visibility == Visibility::Private {
                "priv "
            } else {
                ""
            }
        )
    }
}

struct Variants<'a, T>(&'a Vec<(String, Vec<T>)>)
where
    T: Display;

impl<'a, T> Display for Variants<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.0.iter().fold(Ok(()), |acc, (name, parameters)| {
            acc.and_then(|_| {
                write!(
                    indented(f),
                    "\n| {name}{parameters}",
                    parameters = Parameters(parameters)
                )
            })
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            expression::{Expression, Primitive},
            module::Module,
            parameter::Parameter,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn type_alias() {
        assert_eq!(
            f::n::d(f::a::type_("foo", f::n::tx(TypeExpression::Nil))).to_string(),
            "type foo = nil;"
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            f::n::d(f::a::enum_(
                "foo",
                vec![
                    (String::from("fizz"), vec![]),
                    (String::from("buzz"), vec![f::n::tx(TypeExpression::Nil)]),
                ]
            ))
            .to_string(),
            "enum foo =
  | fizz
  | buzz(nil);"
        );
    }

    #[test]
    fn constant_no_typedef() {
        assert_eq!(
            f::n::d(f::a::const_(
                "foo",
                None,
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "const foo = nil;"
        );
    }

    #[test]
    fn constant_with_typedef() {
        assert_eq!(
            f::n::d(f::a::const_(
                "foo",
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "const foo: nil = nil;"
        );
    }

    #[test]
    fn function_no_parameters() {
        assert_eq!(
            f::n::d(f::a::func_(
                "foo",
                vec![],
                None,
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "func foo -> nil;"
        );
    }

    #[test]
    fn function_with_parameters() {
        assert_eq!(
            f::n::d(f::a::func_(
                "foo",
                vec![f::n::p(Parameter::new(String::from("bar"), None, None))],
                None,
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "func foo(bar) -> nil;"
        );
    }

    #[test]
    fn function_with_return_type() {
        assert_eq!(
            f::n::d(f::a::func_(
                "foo",
                vec![],
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "func foo: nil -> nil;"
        );
    }

    #[test]
    fn view_no_parameters() {
        assert_eq!(
            f::n::d(f::a::view(
                "foo",
                vec![],
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "view foo -> nil;"
        );
    }

    #[test]
    fn view_with_parameters() {
        assert_eq!(
            f::n::d(f::a::view(
                "foo",
                vec![f::n::p(Parameter::new(String::from("bar"), None, None)),],
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "view foo(bar) -> nil;"
        );
    }

    #[test]
    fn empty_module() {
        assert_eq!(
            f::n::d(f::a::module("foo", f::n::mr(Module::new(vec![], vec![])))).to_string(),
            "module foo {}"
        );
    }

    #[test]
    fn module_with_entries() {
        assert_eq!(
            f::n::d(f::a::module(
                "foo",
                f::n::mr(Module::new(
                    vec![],
                    vec![f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Nil)))]
                ))
            ))
            .to_string(),
            "module foo {
  type bar = nil;
}"
        );
    }
}
