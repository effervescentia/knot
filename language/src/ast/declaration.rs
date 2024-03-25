use super::walk;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Binding(pub String);

impl<Visitor> walk::Walk<Visitor> for walk::Span<Binding>
where
    Visitor: walk::Visit,
{
    type Output = Visitor::Binding;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(x, range) = self;

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

impl<Visitor, Binding> walk::Walk<Visitor> for Storage<Binding>
where
    Visitor: walk::Visit,
    Binding: walk::Walk<Visitor, Output = Visitor::Binding>,
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

impl<Visitor, Binding, Expression, TypeExpression> walk::Walk<Visitor>
    for walk::Span<Parameter<Binding, Expression, TypeExpression>>
where
    Visitor: walk::Visit,
    Binding: walk::Walk<Visitor, Output = Visitor::Binding>,
    Expression: walk::Walk<Visitor, Output = Visitor::Expression>,
    TypeExpression: walk::Walk<Visitor, Output = Visitor::TypeExpression>,
{
    type Output = Visitor::Parameter;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(
            super::Parameter {
                binding,
                value_type,
                default_value,
            },
            range,
        ) = self;
        let (binding, v) = binding.walk(v);
        let (value_type, v) = value_type.walk(v);
        let (default_value, v) = default_value.walk(v);

        v.parameter(
            super::Parameter {
                binding,
                value_type,
                default_value,
            },
            range,
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

impl<Visitor, Binding, Expression, TypeExpression, Parameter, Module> walk::Walk<Visitor>
    for walk::Span<Declaration<Binding, Expression, TypeExpression, Parameter, Module>>
where
    Visitor: walk::Visit,
    Binding: walk::Walk<Visitor, Output = Visitor::Binding>,
    Expression: walk::Walk<Visitor, Output = Visitor::Expression>,
    TypeExpression: walk::Walk<Visitor, Output = Visitor::TypeExpression>,
    Parameter: walk::Walk<Visitor, Output = Visitor::Parameter>,
    Module: walk::Walk<Visitor, Output = Visitor::Module>,
{
    type Output = Visitor::Declaration;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(value, range) = self;

        match value {
            super::Declaration::TypeAlias { storage, value } => {
                let (storage, v) = storage.walk(v);
                let (value, v) = value.walk(v);

                v.declaration(super::Declaration::TypeAlias { storage, value }, range)
            }

            super::Declaration::Constant {
                storage,
                value_type,
                value,
            } => {
                let (storage, v) = storage.walk(v);
                let (value_type, v) = value_type.walk(v);
                let (value, v) = value.walk(v);

                v.declaration(
                    super::Declaration::Constant {
                        storage,
                        value_type,
                        value,
                    },
                    range,
                )
            }

            super::Declaration::Enumerated { storage, variants } => {
                let (storage, v) = storage.walk(v);
                let (variants, v) = variants.walk(v);

                v.declaration(super::Declaration::Enumerated { storage, variants }, range)
            }

            super::Declaration::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                let (storage, v) = storage.walk(v);
                let (parameters, v) = parameters.walk(v);
                let (body_type, v) = body_type.walk(v);
                let (body, v) = body.walk(v);

                v.declaration(
                    super::Declaration::Function {
                        storage,
                        parameters,
                        body_type,
                        body,
                    },
                    range,
                )
            }

            super::Declaration::View {
                storage,
                parameters,
                body,
            } => {
                let (storage, v) = storage.walk(v);
                let (parameters, v) = parameters.walk(v);
                let (body, v) = body.walk(v);

                v.declaration(
                    super::Declaration::View {
                        storage,
                        parameters,
                        body,
                    },
                    range,
                )
            }

            super::Declaration::Module { storage, value } => {
                let (storage, v) = storage.walk(v);
                let (value, v) = value.walk(v);

                v.declaration(super::Declaration::Module { storage, value }, range)
            }
        }
    }
}
