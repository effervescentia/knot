use crate::{
    walk::{Span, Visit, Walk},
    Range,
};
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

impl<Visitor, Meta, TypeExpression_> Walk<Visitor, (Range, Meta)>
    for Span<TypeExpression<TypeExpression_>, Meta>
where
    Visitor: Visit<(Range, Meta)>,
    TypeExpression_: Walk<Visitor, (Range, Meta), Output = Visitor::TypeExpression>,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            super::TypeExpression::Primitive(x) => {
                v.type_expression(super::TypeExpression::Primitive(x), ctx)
            }

            super::TypeExpression::Identifier(x) => {
                v.type_expression(super::TypeExpression::Identifier(x), ctx)
            }

            super::TypeExpression::Group(x) => {
                let (x, v) = x.walk(v);

                v.type_expression(super::TypeExpression::Group(Box::new(x)), ctx)
            }

            super::TypeExpression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::PropertyAccess(Box::new(x), property),
                    ctx,
                )
            }

            super::TypeExpression::Function(parameters, x) => {
                let (parameters, v) = parameters.walk(v);
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::Function(parameters, Box::new(x)),
                    ctx,
                )
            }
        }
    }
}
