use super::{Parameters, Typedef};
use crate::ast;
use kore::format::{indented, Block, Indented};
use std::fmt::{Display, Formatter, Write};

impl<Binding, Expression, TypeExpression, Parameter, Module> Display
    for ast::Declaration<Binding, Expression, TypeExpression, Parameter, Module>
where
    Binding: Display,
    Expression: Display,
    TypeExpression: Display,
    Parameter: Display,
    Module: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::TypeAlias { storage, value } => {
                write!(
                    f,
                    "{storage} = {value};",
                    storage = Storage("type", storage)
                )
            }

            Self::Enumerated { storage, variants } => {
                write!(
                    f,
                    "{storage} ={variants};",
                    storage = Storage("enum", storage),
                    variants = Variants(variants)
                )
            }

            Self::Constant {
                storage,
                value_type,
                value,
            } => {
                write!(
                    f,
                    "{storage}{typedef} = {value};",
                    storage = Storage("const", storage),
                    typedef = Typedef(value_type)
                )
            }

            Self::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                write!(
                    f,
                    "{storage}{parameters}{typedef} -> {body};",
                    parameters = Parameters(parameters),
                    storage = Storage("func", storage),
                    typedef = Typedef(body_type)
                )
            }

            Self::View {
                storage,
                parameters,
                body,
            } => {
                write!(
                    f,
                    "{storage}{parameters} -> {body};",
                    storage = Storage("view", storage),
                    parameters = Parameters(parameters)
                )
            }

            Self::Module { storage, value } => {
                write!(
                    f,
                    "{storage} {{{module}}}",
                    storage = Storage("module", storage),
                    module = Indented(Block(value))
                )
            }
        }
    }
}

struct Storage<'a, Binding>(&'a str, &'a ast::Storage<Binding>);

impl<'a, Binding> Display for Storage<'a, Binding>
where
    Binding: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let Self(
            keyword,
            ast::Storage {
                visibility,
                binding,
            },
        ) = self;

        write!(
            f,
            "{visibility}{keyword} {binding}",
            visibility = if visibility == &ast::Visibility::Private {
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
        self.0.iter().try_fold((), |_, (name, parameters)| {
            write!(
                indented(f),
                "\n| {name}{parameters}",
                parameters = Parameters(parameters)
            )
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn type_alias() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::type_alias(
                ast::Storage::public(str!("foo")),
                ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
            ))
            .to_string(),
            "type foo = nil;"
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::enumerated(
                ast::Storage::public(str!("foo")),
                vec![
                    (str!("fizz"), vec![]),
                    (
                        str!("buzz"),
                        vec![ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Nil
                        ))]
                    ),
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
            ast::shape::Declaration(ast::Declaration::constant(
                ast::Storage::public(str!("foo")),
                None,
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "const foo = nil;"
        );
    }

    #[test]
    fn constant_with_typedef() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::constant(
                ast::Storage::public(str!("foo")),
                Some(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "const foo: nil = nil;"
        );
    }

    #[test]
    fn function_no_parameters() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![],
                None,
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "func foo -> nil;"
        );
    }

    #[test]
    fn function_with_parameters() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![ast::shape::Parameter(ast::Parameter::new(
                    str!("bar"),
                    None,
                    None
                ))],
                None,
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "func foo(bar) -> nil;"
        );
    }

    #[test]
    fn function_with_return_type() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![],
                Some(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "func foo: nil -> nil;"
        );
    }

    #[test]
    fn view_no_parameters() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::view(
                ast::Storage::public(str!("foo")),
                vec![],
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "view foo -> nil;"
        );
    }

    #[test]
    fn view_with_parameters() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::view(
                ast::Storage::public(str!("foo")),
                vec![ast::shape::Parameter(ast::Parameter::new(
                    str!("bar"),
                    None,
                    None
                )),],
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "view foo(bar) -> nil;"
        );
    }

    #[test]
    fn empty_module() {
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
    fn module_with_entries() {
        assert_eq!(
            ast::shape::Declaration(ast::Declaration::module(
                ast::Storage::public(str!("foo")),
                ast::shape::Module(ast::Module::new(
                    vec![],
                    vec![ast::shape::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("bar")),
                        ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Nil
                        ))
                    ))]
                ))
            ))
            .to_string(),
            "module foo {
  type bar = nil;
}"
        );
    }
}
