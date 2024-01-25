use crate::ast;
use kore::format::SeparateEach;
use std::fmt::{Display, Formatter};

impl<TypeExpression_> Display for ast::TypeExpression<TypeExpression_>
where
    TypeExpression_: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Primitive(ast::TypePrimitive::Nil) => write!(f, "nil"),
            Self::Primitive(ast::TypePrimitive::Boolean) => write!(f, "boolean"),
            Self::Primitive(ast::TypePrimitive::Integer) => write!(f, "integer"),
            Self::Primitive(ast::TypePrimitive::Float) => write!(f, "float"),
            Self::Primitive(ast::TypePrimitive::String) => write!(f, "string"),
            Self::Primitive(ast::TypePrimitive::Style) => write!(f, "style"),
            Self::Primitive(ast::TypePrimitive::Element) => write!(f, "element"),

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
        }
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
}
