use crate::ast;
use kore::format::{indented, SeparateEach, TerminateEach};
use std::fmt::{Display, Formatter, Write};

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

            Self::Function(parameters, result) => {
                write!(
                    f,
                    "({parameters}) -> {result}",
                    parameters = SeparateEach(", ", parameters)
                )
            }

            Self::Object(entries) if entries.is_empty() => {
                write!(f, "{{}}")
            }

            Self::Object(entries) => {
                writeln!(f, "{{")?;
                write!(
                    indented(f),
                    "{entries}",
                    entries = TerminateEach(",\n", entries)
                )?;
                write!(f, "}}")
            }
        }
    }
}

impl<Binding, TypeExpression> Display for ast::TypeDeclaration<Binding, TypeExpression>
where
    Binding: Display,
    TypeExpression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::TypeAlias { binding, value } => write!(f, "type {binding} = {value};"),

            Self::View {
                binding,
                parameters,
            } => write!(
                f,
                "view {binding} ({parameters});",
                parameters = SeparateEach(", ", parameters)
            ),
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
            declarations = TerminateEach("\n", &self.declarations)
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn nil() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
                .to_string(),
            "nil"
        );
    }

    #[test]
    fn boolean() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean))
                .to_string(),
            "boolean"
        );
    }

    #[test]
    fn integer() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Integer))
                .to_string(),
            "integer"
        );
    }

    #[test]
    fn float() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Float))
                .to_string(),
            "float"
        );
    }

    #[test]
    fn string() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::String))
                .to_string(),
            "string"
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Style))
                .to_string(),
            "style"
        );
    }

    #[test]
    fn element() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Element))
                .to_string(),
            "element"
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Identifier(str!("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Group(Box::new(
                ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
            )))
            .to_string(),
            "(nil)"
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
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
        assert_eq!(
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
        assert_eq!(
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
        assert_eq!(
            ast::shape::TypeExpression(ast::TypeExpression::Object(vec![])).to_string(),
            "{}"
        );
    }

    #[test]
    fn object() {
        assert_eq!(
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

    #[test]
    fn type_alias() {
        assert_eq!(
            ast::shape::TypeDeclaration(ast::TypeDeclaration::type_alias(
                str!("foo"),
                ast::shape::TypeExpression(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil))
            ))
            .to_string(),
            "type foo = nil;"
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            ast::shape::TypeDeclaration(ast::TypeDeclaration::view(
                str!("Foo"),
                vec![
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil
                    )),
                    ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Boolean
                    ))
                ]
            ))
            .to_string(),
            "view Foo (nil, boolean);"
        );
    }

    #[test]
    fn module() {
        assert_eq!(
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
                        vec![
                            ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Nil
                            )),
                            ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Boolean
                            ))
                        ]
                    ))
                ]
            })
            .to_string(),
            "type foo = nil;
view Bar (nil, boolean);
"
        );
    }
}
