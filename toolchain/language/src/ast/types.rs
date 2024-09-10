use super::IsEmpty;
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
pub enum TypeDeclaration<Binding, TypeExpression, TypeModule> {
    TypeAlias {
        binding: Binding,
        value: TypeExpression,
    },

    Enumerated {
        binding: Binding,
        variants: Vec<(String, Vec<TypeExpression>)>,
    },

    View {
        binding: Binding,
        attributes: TypeExpression,
    },

    Function {
        binding: Binding,
        parameters: Vec<TypeExpression>,
        result: TypeExpression,
    },

    Module {
        binding: Binding,
        module: TypeModule,
    },
}

impl<Binding, TypeExpression, TypeModule> TypeDeclaration<Binding, TypeExpression, TypeModule> {
    pub const fn type_alias(binding: Binding, value: TypeExpression) -> Self {
        Self::TypeAlias { binding, value }
    }

    pub const fn enumerated(
        binding: Binding,
        variants: Vec<(String, Vec<TypeExpression>)>,
    ) -> Self {
        Self::Enumerated { binding, variants }
    }

    pub const fn view(binding: Binding, attributes: TypeExpression) -> Self {
        Self::View {
            binding,
            attributes,
        }
    }

    pub const fn function(
        binding: Binding,
        parameters: Vec<TypeExpression>,
        result: TypeExpression,
    ) -> Self {
        Self::Function {
            binding,
            parameters,
            result,
        }
    }

    pub const fn module(binding: Binding, module: TypeModule) -> Self {
        Self::Module { binding, module }
    }

    pub const fn binding(&self) -> &Binding {
        match self {
            Self::TypeAlias { binding, .. }
            | Self::Enumerated { binding, .. }
            | Self::View { binding, .. }
            | Self::Function { binding, .. }
            | Self::Module { binding, .. } => binding,
        }
    }
}

impl<Visitor, Context, Binding, TypeExpression, TypeModule> Walk<Visitor>
    for (
        TypeDeclaration<Binding, TypeExpression, TypeModule>,
        Context,
    )
where
    Visitor: TypingsVisitor<Context = Context>,
    Binding: Walk<Visitor, Output = Visitor::Binding>,
    TypeExpression: Walk<Visitor, Output = Visitor::TypeExpression>,
    TypeModule: Walk<Visitor, Output = Visitor::TypeModule>,
{
    type Output = Visitor::TypeDeclaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            TypeDeclaration::TypeAlias { binding, value } => {
                let ((binding, value), v) = (binding, value).walk_each(v);

                v.type_declaration(TypeDeclaration::TypeAlias { binding, value }, ctx)
            }

            TypeDeclaration::Enumerated { binding, variants } => {
                let ((binding, variants), v) = (binding, variants).walk_each(v);

                v.type_declaration(TypeDeclaration::Enumerated { binding, variants }, ctx)
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

            TypeDeclaration::Function {
                binding,
                parameters,
                result,
            } => {
                let ((binding, parameters, result), v) = (binding, parameters, result).walk_each(v);

                v.type_declaration(
                    TypeDeclaration::Function {
                        binding,
                        parameters,
                        result,
                    },
                    ctx,
                )
            }

            TypeDeclaration::Module { binding, module } => {
                let ((binding, module), v) = (binding, module).walk_each(v);

                v.type_declaration(TypeDeclaration::Module { binding, module }, ctx)
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TypeModule<Import, TypeDeclaration> {
    pub imports: Vec<Import>,
    pub declarations: Vec<TypeDeclaration>,
}

impl<Import, TypeDeclaration> TypeModule<Import, TypeDeclaration> {
    pub fn new(imports: Vec<Import>, declarations: Vec<TypeDeclaration>) -> Self {
        Self {
            imports,
            declarations,
        }
    }
}

impl<Import, TypeDeclaration> IsEmpty for TypeModule<Import, TypeDeclaration> {
    fn is_empty(&self) -> bool {
        self.declarations.is_empty()
    }
}

impl<Visitor, Context, Import, TypeDeclaration> Walk<Visitor>
    for (TypeModule<Import, TypeDeclaration>, Context)
where
    Visitor: TypingsVisitor<Context = Context>,
    Import: Walk<Visitor, Output = Visitor::Import>,
    TypeDeclaration: Walk<Visitor, Output = Visitor::TypeDeclaration>,
{
    type Output = Visitor::TypeModule;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            TypeModule {
                imports,
                declarations,
            },
            ctx,
        ) = self;
        let ((imports, declarations), v) = (imports, declarations).walk_each(v);

        v.type_module(
            TypeModule {
                imports,
                declarations,
            },
            ctx,
        )
    }
}
