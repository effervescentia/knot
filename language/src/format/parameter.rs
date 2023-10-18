use super::Typedef;
use crate::ast::{Parameter, ParameterNode};
use std::fmt::{Display, Formatter};

impl<R, C> Display for ParameterNode<R, C>
where
    R: Clone,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let Parameter {
            name,
            value_type,
            default_value,
        } = self.node().value();

        write!(
            f,
            "{name}{typedef}{default}",
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
    use crate::{
        ast::{Expression, Parameter, Primitive, TypeExpression},
        test::fixture as f,
    };

    #[test]
    fn untyped() {
        assert_eq!(
            f::n::p(Parameter::new(String::from("foo"), None, None)).to_string(),
            "foo"
        );
    }

    #[test]
    fn with_typedef() {
        assert_eq!(
            f::n::p(Parameter::new(
                String::from("foo"),
                Some(f::n::tx(TypeExpression::Nil)),
                None
            ))
            .to_string(),
            "foo: nil"
        );
    }

    #[test]
    fn with_default() {
        assert_eq!(
            f::n::p(Parameter::new(
                String::from("foo"),
                None,
                Some(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "foo = nil"
        );
    }

    #[test]
    fn with_typedef_and_default() {
        assert_eq!(
            f::n::p(Parameter::new(
                String::from("foo"),
                Some(f::n::tx(TypeExpression::Nil)),
                Some(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "foo: nil = nil"
        );
    }
}
