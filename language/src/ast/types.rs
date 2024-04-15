use crate::walk::{CommonVisitor, TypingsVisitor, Walk, WalkEach};
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
pub enum ObjectTypeExpressionEntry<Binding, TypeExpression> {
    Required(Binding, TypeExpression),
    Optional(Binding, TypeExpression),
    Spread(TypeExpression),
}

impl<Binding, TypeExpression> ObjectTypeExpressionEntry<Binding, TypeExpression> {
    pub const fn binding(&self) -> Option<&Binding> {
        match self {
            Self::Required(binding, _) | Self::Optional(binding, _) => Some(binding),

            Self::Spread(_) => None,
        }
    }
}

impl<Visitor, Context, Binding, TypeExpression> Walk<Visitor>
    for ObjectTypeExpressionEntry<Binding, TypeExpression>
where
    Visitor: CommonVisitor<Context = Context>,
    Binding: Walk<Visitor, Output = Visitor::Binding>,
    TypeExpression: Walk<Visitor, Output = Visitor::TypeExpression>,
{
    type Output = ObjectTypeExpressionEntry<Visitor::Binding, Visitor::TypeExpression>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        match self {
            Self::Required(binding, x) => {
                let ((binding, x), v) = (binding, x).walk_each(v);

                (ObjectTypeExpressionEntry::Required(binding, x), v)
            }

            Self::Optional(binding, x) => {
                let ((binding, x), v) = (binding, x).walk_each(v);

                (ObjectTypeExpressionEntry::Optional(binding, x), v)
            }

            Self::Spread(x) => {
                let (x, v) = x.walk(v);

                (ObjectTypeExpressionEntry::Spread(x), v)
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression<Binding, TypeExpression_> {
    Primitive(TypePrimitive),
    Identifier(String),
    Group(Box<TypeExpression_>),
    PropertyAccess(Box<TypeExpression_>, String),
    Function(Vec<TypeExpression_>, Box<TypeExpression_>),
    Object(Vec<ObjectTypeExpressionEntry<Binding, TypeExpression_>>),
}

impl<Visitor, Context, Binding, TypeExpression_> Walk<Visitor>
    for (TypeExpression<Binding, TypeExpression_>, Context)
where
    Visitor: CommonVisitor<Context = Context>,
    Binding: Walk<Visitor, Output = Visitor::Binding>,
    TypeExpression_: Walk<Visitor, Output = Visitor::TypeExpression>,
{
    type Output = Visitor::TypeExpression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            TypeExpression::Primitive(x) => v.type_expression(TypeExpression::Primitive(x), ctx),

            TypeExpression::Identifier(x) => v.type_expression(TypeExpression::Identifier(x), ctx),

            TypeExpression::Group(x) => {
                let (x, v) = x.walk(v);

                v.type_expression(TypeExpression::Group(Box::new(x)), ctx)
            }

            TypeExpression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.type_expression(TypeExpression::PropertyAccess(Box::new(x), property), ctx)
            }

            TypeExpression::Function(parameters, x) => {
                let (parameters, v) = parameters.walk(v);
                let (x, v) = x.walk(v);

                v.type_expression(TypeExpression::Function(parameters, Box::new(x)), ctx)
            }

            TypeExpression::Object(entries) => {
                let (entries, v) = entries.walk(v);

                v.type_expression(TypeExpression::Object(entries), ctx)
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
        attributes: TypeExpression,
    },
}

impl<Binding, TypeExpression> TypeDeclaration<Binding, TypeExpression> {
    pub const fn type_alias(binding: Binding, value: TypeExpression) -> Self {
        Self::TypeAlias { binding, value }
    }

    pub const fn view(binding: Binding, attributes: TypeExpression) -> Self {
        Self::View {
            binding,
            attributes,
        }
    }

    pub const fn binding(&self) -> &Binding {
        match self {
            Self::TypeAlias { binding, .. } | Self::View { binding, .. } => binding,
        }
    }
}

impl<Visitor, Context, Binding, TypeExpression> Walk<Visitor>
    for (TypeDeclaration<Binding, TypeExpression>, Context)
where
    Visitor: TypingsVisitor<Context = Context>,
    Binding: Walk<Visitor, Output = Visitor::Binding>,
    TypeExpression: Walk<Visitor, Output = Visitor::TypeExpression>,
{
    type Output = Visitor::TypeDeclaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            TypeDeclaration::TypeAlias { binding, value } => {
                let ((binding, value), v) = (binding, value).walk_each(v);

                v.type_declaration(TypeDeclaration::TypeAlias { binding, value }, ctx)
            }

            TypeDeclaration::View {
                binding,
                attributes,
            } => {
                let ((binding, attributes), v) = (binding, attributes).walk_each(v);

                v.type_declaration(
                    TypeDeclaration::View {
                        binding,
                        attributes,
                    },
                    ctx,
                )
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TypeModule<TypeDeclaration> {
    pub declarations: Vec<TypeDeclaration>,
}

impl<TypeDeclaration> TypeModule<TypeDeclaration> {
    pub fn new(declarations: Vec<TypeDeclaration>) -> Self {
        Self { declarations }
    }
}

impl<Visitor, Context, TypeDeclaration> Walk<Visitor> for (TypeModule<TypeDeclaration>, Context)
where
    Visitor: TypingsVisitor<Context = Context>,
    TypeDeclaration: Walk<Visitor, Output = Visitor::TypeDeclaration>,
{
    type Output = Visitor::TypeModule;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (TypeModule { declarations }, ctx) = self;
        let (declarations, v) = declarations.walk(v);

        v.type_module(TypeModule { declarations }, ctx)
    }
}
