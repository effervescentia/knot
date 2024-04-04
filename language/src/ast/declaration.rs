use crate::{
    walk::{Span, Visit, Walk, WalkEach},
    Range,
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Binding(pub String);

impl<Visitor, Meta> Walk<Visitor, (Range, Meta)> for Span<Binding, ()>
where
    Visitor: Visit<(Range, Meta)>,
{
    type Output = Visitor::Binding;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (x, (range, _)) = self;

        v.binding(x, range)
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Visibility {
    Public,
    Private,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Storage<Binding> {
    pub visibility: Visibility,
    pub binding: Binding,
}

impl<Binding> Storage<Binding> {
    pub const fn new(visibility: Visibility, binding: Binding) -> Self {
        Self {
            visibility,
            binding,
        }
    }

    pub const fn public(binding: Binding) -> Self {
        Self::new(Visibility::Public, binding)
    }

    pub const fn private(binding: Binding) -> Self {
        Self::new(Visibility::Private, binding)
    }
}

impl<Visitor, Meta, Binding> Walk<Visitor, (Range, Meta)> for Storage<Binding>
where
    Visitor: Visit<(Range, Meta)>,
    Binding: Walk<Visitor, (Range, Meta), Output = Visitor::Binding>,
{
    type Output = Storage<Visitor::Binding>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (binding, v) = self.binding.walk(v);

        (
            Storage {
                visibility: self.visibility,
                binding,
            },
            v,
        )
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter<Binding, Expression, TypeExpression> {
    pub binding: Binding,
    pub value_type: Option<TypeExpression>,
    pub default_value: Option<Expression>,
}

impl<Binding, Expression, TypeExpression> Parameter<Binding, Expression, TypeExpression> {
    pub const fn new(
        binding: Binding,
        value_type: Option<TypeExpression>,
        default_value: Option<Expression>,
    ) -> Self {
        Self {
            binding,
            value_type,
            default_value,
        }
    }
}

impl<Visitor, Meta, Binding, Expression, TypeExpression> Walk<Visitor, (Range, Meta)>
    for Span<Parameter<Binding, Expression, TypeExpression>, Meta>
where
    Visitor: Visit<(Range, Meta)>,
    Binding: Walk<Visitor, (Range, Meta), Output = Visitor::Binding>,
    Expression: Walk<Visitor, (Range, Meta), Output = Visitor::Expression>,
    TypeExpression: Walk<Visitor, (Range, Meta), Output = Visitor::TypeExpression>,
{
    type Output = Visitor::Parameter;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            super::Parameter {
                binding,
                value_type,
                default_value,
            },
            ctx,
        ) = self;
        let ((binding, value_type, default_value), v) =
            (binding, value_type, default_value).walk_each(v);

        v.parameter(
            super::Parameter {
                binding,
                value_type,
                default_value,
            },
            ctx,
        )
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Declaration<Binding, Expression, TypeExpression, Parameter, Module> {
    TypeAlias {
        storage: Storage<Binding>,
        value: TypeExpression,
    },

    Constant {
        storage: Storage<Binding>,
        value_type: Option<TypeExpression>,
        value: Expression,
    },

    Enumerated {
        storage: Storage<Binding>,
        variants: Vec<(String, Vec<TypeExpression>)>,
    },

    Function {
        storage: Storage<Binding>,
        parameters: Vec<Parameter>,
        body_type: Option<TypeExpression>,
        body: Expression,
    },

    View {
        storage: Storage<Binding>,
        parameters: Vec<Parameter>,
        body: Expression,
    },

    Module {
        storage: Storage<Binding>,
        value: Module,
    },
}

impl<Binding, Expression, TypeExpression, Parameter, Module>
    Declaration<Binding, Expression, TypeExpression, Parameter, Module>
{
    pub const fn type_alias(storage: Storage<Binding>, value: TypeExpression) -> Self {
        Self::TypeAlias { storage, value }
    }

    pub const fn constant(
        storage: Storage<Binding>,
        value_type: Option<TypeExpression>,
        value: Expression,
    ) -> Self {
        Self::Constant {
            storage,
            value_type,
            value,
        }
    }

    pub const fn enumerated(
        storage: Storage<Binding>,
        variants: Vec<(String, Vec<TypeExpression>)>,
    ) -> Self {
        Self::Enumerated { storage, variants }
    }

    pub const fn function(
        storage: Storage<Binding>,
        parameters: Vec<Parameter>,
        body_type: Option<TypeExpression>,
        body: Expression,
    ) -> Self {
        Self::Function {
            storage,
            parameters,
            body_type,
            body,
        }
    }

    pub const fn view(
        storage: Storage<Binding>,
        parameters: Vec<Parameter>,
        body: Expression,
    ) -> Self {
        Self::View {
            storage,
            parameters,
            body,
        }
    }

    pub const fn module(storage: Storage<Binding>, value: Module) -> Self {
        Self::Module { storage, value }
    }

    pub const fn binding(&self) -> &Binding {
        match self {
            Self::TypeAlias { storage, .. }
            | Self::Constant { storage, .. }
            | Self::Enumerated { storage, .. }
            | Self::Function { storage, .. }
            | Self::View { storage, .. }
            | Self::Module { storage, .. } => &storage.binding,
        }
    }

    pub const fn visibility(&self) -> Visibility {
        match self {
            Self::TypeAlias { storage, .. }
            | Self::Constant { storage, .. }
            | Self::Enumerated { storage, .. }
            | Self::Function { storage, .. }
            | Self::View { storage, .. }
            | Self::Module { storage, .. } => storage.visibility,
        }
    }

    pub fn is_public(&self) -> bool {
        self.visibility() == Visibility::Public
    }
}

impl<Visitor, Meta, Binding, Expression, TypeExpression, Parameter, Module>
    Walk<Visitor, (Range, Meta)>
    for Span<Declaration<Binding, Expression, TypeExpression, Parameter, Module>, Meta>
where
    Visitor: Visit<(Range, Meta)>,
    Binding: Walk<Visitor, (Range, Meta), Output = Visitor::Binding>,
    Expression: Walk<Visitor, (Range, Meta), Output = Visitor::Expression>,
    TypeExpression: Walk<Visitor, (Range, Meta), Output = Visitor::TypeExpression>,
    Parameter: Walk<Visitor, (Range, Meta), Output = Visitor::Parameter>,
    Module: Walk<Visitor, (Range, Meta), Output = Visitor::Module>,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            super::Declaration::TypeAlias { storage, value } => {
                let ((storage, value), v) = v.scoped(|v| (storage, value).walk_each(v));

                v.declaration(super::Declaration::TypeAlias { storage, value }, ctx)
            }

            super::Declaration::Constant {
                storage,
                value_type,
                value,
            } => {
                let ((storage, value_type, value), v) =
                    v.scoped(|v| (storage, value_type, value).walk_each(v));

                v.declaration(
                    super::Declaration::Constant {
                        storage,
                        value_type,
                        value,
                    },
                    ctx,
                )
            }

            super::Declaration::Enumerated { storage, variants } => {
                let ((storage, variants), v) = v.scoped(|v| (storage, variants).walk_each(v));

                v.declaration(super::Declaration::Enumerated { storage, variants }, ctx)
            }

            super::Declaration::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                let ((storage, parameters, body_type, body), v) =
                    v.scoped(|v| (storage, parameters, body_type, body).walk_each(v));

                v.declaration(
                    super::Declaration::Function {
                        storage,
                        parameters,
                        body_type,
                        body,
                    },
                    ctx,
                )
            }

            super::Declaration::View {
                storage,
                parameters,
                body,
            } => {
                let ((storage, parameters, body), v) =
                    v.scoped(|v| (storage, parameters, body).walk_each(v));

                v.declaration(
                    super::Declaration::View {
                        storage,
                        parameters,
                        body,
                    },
                    ctx,
                )
            }

            super::Declaration::Module { storage, value } => {
                let ((storage, value), v) = v.scoped(|v| (storage, value).walk_each(v));

                v.declaration(super::Declaration::Module { storage, value }, ctx)
            }
        }
    }
}
