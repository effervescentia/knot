use super::Typedef;
use crate::ast;
use std::fmt::{Display, Formatter};

impl<Binding, Expression, TypeExpression> Display
    for ast::Parameter<Binding, Expression, TypeExpression>
where
    Binding: Display,
    Expression: Display,
    TypeExpression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let Self {
            binding,
            value_type,
            default_value,
        } = self;

        write!(
            f,
            "{binding}{typedef}{default}",
            typedef = Typedef(value_type),
            default = FormatDefault(default_value)
        )
    }
}

struct FormatDefault<'a, T>(&'a Option<T>)
where
    T: Display;

impl<'a, T> Display for FormatDefault<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if let Some(value) = self.0 {
            write!(f, " = {value}")
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
    fn untyped() {
        assert_eq!(
            ast::shape::Parameter(ast::Parameter::new(str!("foo"), None, None)).to_string(),
            "foo"
        );
    }

    #[test]
    fn with_typedef() {
        assert_eq!(
            ast::shape::Parameter(ast::Parameter::new(
                str!("foo"),
                Some(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
                None
            ))
            .to_string(),
            "foo: nil"
        );
    }

    #[test]
    fn with_default() {
        assert_eq!(
            ast::shape::Parameter(ast::Parameter::new(
                str!("foo"),
                None,
                Some(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "foo = nil"
        );
    }

    #[test]
    fn with_typedef_and_default() {
        assert_eq!(
            ast::shape::Parameter(ast::Parameter::new(
                str!("foo"),
                Some(ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil
                ))),
                Some(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "foo: nil = nil"
        );
    }
}
