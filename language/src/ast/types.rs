use crate::walk::{Visit, Walk};
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
}

impl<Visitor, Context, TypeExpression_> Walk<Visitor> for (TypeExpression<TypeExpression_>, Context)
where
    Visitor: Visit<Context = Context>,
    TypeExpression_: Walk<Visitor, Output = Visitor::TypeExpression>,
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

#[derive(Clone, Debug, PartialEq)]
pub enum TypeDeclaration<Binding, TypeExpression> {
    TypeAlias {
        binding: Binding,
        value: TypeExpression,
    },

    View {
        binding: Binding,
        parameters: Vec<TypeExpression>,
    },
}

impl<Binding, TypeExpression> TypeDeclaration<Binding, TypeExpression> {
    pub const fn type_alias(binding: Binding, value: TypeExpression) -> Self {
        Self::TypeAlias { binding, value }
    }

    pub const fn view(binding: Binding, parameters: Vec<TypeExpression>) -> Self {
        Self::View {
            binding,
            parameters,
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TypeModule<Declaration> {
    pub declarations: Vec<Declaration>,
}

impl<Declaration> TypeModule<Declaration> {
    pub fn new(declarations: Vec<Declaration>) -> Self {
        Self { declarations }
    }
}

// impl<Visitor, Context, Declaration> Walk<Visitor> for (TypeModule<Declaration>, Context)
// where
//     Visitor: Visit<Context = Context>,
//     Import: Walk<Visitor, Output = Visitor::Import>,
//     Declaration: Walk<Visitor, Output = Visitor::Declaration>,
// {
//     type Output = Visitor::Module;

//     fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
//         let (
//             super::Module {
//                 imports,
//                 declarations,
//             },
//             ctx,
//         ) = self;
//         let ((imports, declarations), v) = (imports, declarations).walk_each(v);

//         v.module(
//             super::Module {
//                 imports,
//                 declarations,
//             },
//             ctx,
//         )
//     }
// }
