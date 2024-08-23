use crate::{
    ast::{self, IsEmpty},
    format::Object,
};
use kore::format::{indented, SuffixEach};
use std::fmt::{Display, Formatter, Write};

use super::{Lambda, Parameters};

impl Display for ast::TypePrimitive {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Nil => write!(f, "nil"),
            Self::Boolean => write!(f, "boolean"),
            Self::Integer => write!(f, "integer"),
            Self::Float => write!(f, "float"),
            Self::String => write!(f, "string"),
            Self::Style => write!(f, "style"),
            Self::Element => write!(f, "element"),
        }
    }
}

impl<Binding, TypeExpression> Display for ast::ObjectTypeExpressionEntry<Binding, TypeExpression>
where
    Binding: Display,
    TypeExpression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Required(binding, x) => write!(f, "{binding}: {x}"),

            Self::Optional(binding, x) => write!(f, "{binding}?: {x}"),

            Self::Spread(x) => write!(f, "...{x}"),
        }
    }
}

impl<Binding, TypeExpression> Display for ast::TypeExpression<Binding, TypeExpression>
where
    Binding: Display,
    TypeExpression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Primitive(x) => write!(f, "{x}"),

            Self::Identifier(x) => write!(f, "{x}"),

            Self::Group(x) => write!(f, "({x})"),

            Self::PropertyAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            Self::Function(parameters, result) => Lambda(parameters, result).fmt(f),

            Self::Object(entries) => Object(entries).fmt(f),
        }
    }
}

impl<Binding, TypeExpression, TypeModule> Display
    for ast::TypeDeclaration<Binding, TypeExpression, TypeModule>
where
    Binding: Display,
    TypeExpression: Display,
    TypeModule: Display + IsEmpty,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::TypeAlias { binding, value } => write!(f, "type {binding} = {value};"),

            Self::View {
                binding,
                attributes,
            } => write!(f, "view {binding} {attributes};"),

            Self::Function {
                binding,
                parameters,
                result,
            } => {
                write!(
                    f,
                    "func {binding}{parameters} -> {result};",
                    parameters = Parameters(parameters)
                )
            }

            Self::Module { binding, module } => {
                write!(f, "module {binding} {{")?;

                if !module.is_empty() {
                    write!(indented(f), "\n{module}")?;
                }

                write!(f, "}}")
            }
        }
    }
}

impl<TypeDeclaration> Display for ast::TypeModule<TypeDeclaration>
where
    TypeDeclaration: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(
            f,
            "{declarations}",
            declarations = SuffixEach("\n", &self.declarations)
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::{assert_str_eq, str};

    #[test]
    fn nil() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
                .to_string(),
            "nil"
        );
    }

    #[test]
    fn boolean() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean))
                .to_string(),
            "boolean"
        );
    }

    #[test]
    fn integer() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Integer))
                .to_string(),
            "integer"
        );
    }

    #[test]
    fn float() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Float))
                .to_string(),
            "float"
        );
    }

    #[test]
    fn string() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::String))
                .to_string(),
            "string"
        );
    }

    #[test]
    fn style() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Style))
                .to_string(),
            "style"
        );
    }

    #[test]
    fn element() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Element))
                .to_string(),
            "element"
        );
    }

    #[test]
    fn identifier() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Identifier(str!("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn group() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Group(Box::new(
                ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
            )))
            .to_string(),
            "(nil)"
        );
    }

    #[test]
    fn property_access() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::PropertyAccess(
                Box::new(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
                str!("foo")
            ))
            .to_string(),
            "nil.foo"
        );
    }

    #[test]
    fn function_no_parameters() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Function(
                vec![],
                Box::new(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
            ))
            .to_string(),
            "() -> nil"
        );
    }

    #[test]
    fn function_with_parameters() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Function(
                vec![
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    )),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    ))
                ],
                Box::new(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
            ))
            .to_string(),
            "(nil, nil) -> nil"
        );
    }

    #[test]
    fn empty_object() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Object(vec![])).to_string(),
            "{}"
        );
    }

    #[test]
    fn object() {
        assert_str_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Object(vec![
                ast::ObjectTypeExpressionEntry::Required(
                    str!("foo"),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Integer
                    ))
                ),
                ast::ObjectTypeExpressionEntry::Optional(
                    str!("bar"),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Boolean
                    ))
                ),
                ast::ObjectTypeExpressionEntry::Spread(ast::shape::TypeExpression(
                    ast::TypeExpression::Identifier(str!("fizz"))
                ))
            ]))
            .to_string(),
            "{
  foo: integer,
  bar?: boolean,
  ...fizz,
}"
        );
    }

    mod type_module {
        use super::*;

        #[test]
        fn type_alias() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::type_alias(
                    str!("foo"),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    ))
                ))
                .to_string(),
                "type foo = nil;"
            );
        }

        #[test]
        fn view() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::view(
                    str!("Foo"),
                    ast::shape::TypeExpression(ast::TypeExpression::Object(vec![
                        ast::ObjectTypeExpressionEntry::Required(
                            str!("bar"),
                            ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Nil
                            ))
                        ),
                        ast::ObjectTypeExpressionEntry::Optional(
                            str!("fizz"),
                            ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Boolean
                            ))
                        )
                    ]))
                ))
                .to_string(),
                "view Foo {
  bar: nil,
  fizz?: boolean,
};"
            );
        }

        #[test]
        fn function_no_parameters() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::function(
                    str!("foo"),
                    vec![],
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::String
                    ))
                ))
                .to_string(),
                "func foo -> string;"
            );
        }

        #[test]
        fn function_with_parameters() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::function(
                    str!("foo"),
                    vec![
                        ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Integer
                        )),
                        ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Boolean
                        ))
                    ],
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::String
                    ))
                ))
                .to_string(),
                "func foo(integer, boolean) -> string;"
            );
        }

        #[test]
        fn empty_module() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::module(
                    str!("foo"),
                    ast::shape::TypeModule(ast::TypeModule {
                        declarations: vec![]
                    })
                ))
                .to_string(),
                "module foo {}"
            );
        }

        #[test]
        fn module() {
            assert_str_eq!(
                ast::shape::TypeDeclaration(ast::TypeDeclaration::module(
                    str!("foo"),
                    ast::shape::TypeModule(ast::TypeModule {
                        declarations: vec![
                            ast::shape::TypeDeclaration(ast::TypeDeclaration::type_alias(
                                str!("bar"),
                                ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                    ast::TypePrimitive::Nil
                                ))
                            )),
                            ast::shape::TypeDeclaration(ast::TypeDeclaration::view(
                                str!("Fizz"),
                                ast::shape::TypeExpression(ast::TypeExpression::Object(vec![]))
                            ))
                        ]
                    })
                ))
                .to_string(),
                "module foo {
  type bar = nil;
  view Fizz {};
}"
            );
        }
    }

    #[test]
    fn module() {
        assert_str_eq!(
            ast::shape::TypeModule(ast::TypeModule {
                declarations: vec![
                    ast::shape::TypeDeclaration(ast::TypeDeclaration::type_alias(
                        str!("foo"),
                        ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Nil
                        ))
                    )),
                    ast::shape::TypeDeclaration(ast::TypeDeclaration::view(
                        str!("Bar"),
                        ast::shape::TypeExpression(ast::TypeExpression::Object(vec![]))
                    ))
                ]
            })
            .to_string(),
            "type foo = nil;
view Bar {};
"
        );
    }
}
