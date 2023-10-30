use crate::ast::{TypeExpression, TypeExpressionNode};
use kore::format::SeparateEach;
use std::fmt::{Display, Formatter};

impl<R, C> Display for TypeExpressionNode<R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self.node().value() {
            TypeExpression::Nil => write!(f, "nil"),
            TypeExpression::Boolean => write!(f, "boolean"),
            TypeExpression::Integer => write!(f, "integer"),
            TypeExpression::Float => write!(f, "float"),
            TypeExpression::String => write!(f, "string"),
            TypeExpression::Style => write!(f, "style"),
            TypeExpression::Element => write!(f, "element"),

            TypeExpression::Identifier(x) => write!(f, "{x}"),

            TypeExpression::Group(x) => write!(f, "({x})"),

            TypeExpression::DotAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            TypeExpression::Function(parameters, result) => {
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
    use crate::{ast::TypeExpression, test::fixture as f};

    #[test]
    fn nil() {
        assert_eq!(f::n::tx(TypeExpression::Nil).to_string(), "nil");
    }

    #[test]
    fn boolean() {
        assert_eq!(f::n::tx(TypeExpression::Boolean).to_string(), "boolean");
    }

    #[test]
    fn integer() {
        assert_eq!(f::n::tx(TypeExpression::Integer).to_string(), "integer");
    }

    #[test]
    fn float() {
        assert_eq!(f::n::tx(TypeExpression::Float).to_string(), "float");
    }

    #[test]
    fn string() {
        assert_eq!(f::n::tx(TypeExpression::String).to_string(), "string");
    }

    #[test]
    fn style() {
        assert_eq!(f::n::tx(TypeExpression::Style).to_string(), "style");
    }

    #[test]
    fn element() {
        assert_eq!(f::n::tx(TypeExpression::Element).to_string(), "element");
    }

    #[test]
    fn identifier() {
        assert_eq!(
            f::n::tx(TypeExpression::Identifier(String::from("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            f::n::tx(TypeExpression::Group(Box::new(f::n::tx(
                TypeExpression::Nil
            ))))
            .to_string(),
            "(nil)"
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            f::n::tx(TypeExpression::DotAccess(
                Box::new(f::n::tx(TypeExpression::Nil)),
                String::from("foo")
            ))
            .to_string(),
            "nil.foo"
        );
    }

    #[test]
    fn function_no_parameters() {
        assert_eq!(
            f::n::tx(TypeExpression::Function(
                vec![],
                Box::new(f::n::tx(TypeExpression::Nil)),
            ))
            .to_string(),
            "() -> nil"
        );
    }

    #[test]
    fn function_with_parameters() {
        assert_eq!(
            f::n::tx(TypeExpression::Function(
                vec![f::n::tx(TypeExpression::Nil), f::n::tx(TypeExpression::Nil)],
                Box::new(f::n::tx(TypeExpression::Nil)),
            ))
            .to_string(),
            "(nil, nil) -> nil"
        );
    }
}
