use super::walk;
use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum TypePrimitive {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
}

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression<TypeExpression_> {
    Primitive(TypePrimitive),
    Identifier(String),
    Group(Box<TypeExpression_>),
    PropertyAccess(Box<TypeExpression_>, String),
    Function(Vec<TypeExpression_>, Box<TypeExpression_>),
    // View(Vec<(String, TypeExpression)>),
}

impl<Visitor, TypeExpression_> walk::Walk<Visitor> for walk::Span<TypeExpression<TypeExpression_>>
where
    Visitor: walk::Visit,
    TypeExpression_: walk::Walk<Visitor, Output = Visitor::TypeExpression>,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor)
    where
        Visitor: walk::Visit,
    {
        let Self(value, range) = self;

        match value {
            super::TypeExpression::Primitive(x) => {
                v.type_expression(super::TypeExpression::Primitive(x), range)
            }

            super::TypeExpression::Identifier(x) => {
                v.type_expression(super::TypeExpression::Identifier(x), range)
            }

            super::TypeExpression::Group(x) => {
                let (x, v) = x.walk(v);

                v.type_expression(super::TypeExpression::Group(Box::new(x)), range)
            }

            super::TypeExpression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::PropertyAccess(Box::new(x), property),
                    range,
                )
            }

            super::TypeExpression::Function(parameters, x) => {
                let (parameters, v) = parameters.walk(v);
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::Function(parameters, Box::new(x)),
                    range,
                )
            }
        }
    }
}
