use super::{
    data::{Data, Inference, Weak},
    ToWeak,
};
use crate::ast;
use lang::{
    types::{Enumerated, Kind, Type},
    Fragment, NodeId,
};

impl ToWeak for Fragment {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::Component(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),
        }
    }
}

impl ToWeak for ast::Expression<NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Primitive(x) => (
                Kind::Value,
                Data::Local(match x {
                    ast::Primitive::Nil => Type::Nil,
                    ast::Primitive::Boolean(..) => Type::Boolean,
                    ast::Primitive::Integer(..) => Type::Integer,
                    ast::Primitive::Float(..) => Type::Float,
                    ast::Primitive::String(..) => Type::String,
                }),
            ),

            Self::Identifier(x) => (Kind::Value, Data::Infer(Inference::Reference(x.clone()))),

            Self::Group(x) => (Kind::Value, Data::Inherit(**x)),

            Self::Closure(xs) => (Kind::Value, {
                match xs.last() {
                    Some(id) => Data::Inherit(*id),
                    None => Data::Local(Type::Nil),
                }
            }),

            Self::UnaryOperation(op, id) => (
                Kind::Value,
                match op {
                    ast::UnaryOperator::Not => Data::Local(Type::Boolean),

                    _ => Data::Inherit(**id),
                },
            ),

            Self::BinaryOperation(op, lhs, rhs) => (
                Kind::Value,
                match op {
                    ast::BinaryOperator::Equal
                    | ast::BinaryOperator::NotEqual
                    | ast::BinaryOperator::And
                    | ast::BinaryOperator::Or
                    | ast::BinaryOperator::LessThan
                    | ast::BinaryOperator::LessThanOrEqual
                    | ast::BinaryOperator::GreaterThan
                    | ast::BinaryOperator::GreaterThanOrEqual => Data::Local(Type::Boolean),

                    ast::BinaryOperator::Divide | ast::BinaryOperator::Exponent => {
                        Data::Local(Type::Float)
                    }

                    ast::BinaryOperator::Add
                    | ast::BinaryOperator::Subtract
                    | ast::BinaryOperator::Multiply => {
                        Data::Infer(Inference::Arithmetic(**lhs, **rhs))
                    }
                },
            ),

            Self::PropertyAccess(x, property) => (
                Kind::Value,
                Data::Infer(Inference::Property(**x, property.clone())),
            ),

            Self::FunctionCall(x, ..) => (Kind::Value, Data::Infer(Inference::FunctionResult(**x))),

            Self::Style(..) => (Kind::Value, Data::Local(Type::Style)),

            Self::Component(..) => (Kind::Value, Data::Local(Type::Element)),
        }
    }
}

impl ToWeak for ast::Statement<NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Expression(id) => (Kind::Value, Data::Inherit(*id)),

            Self::Variable(..) => (Kind::Value, Data::Local(Type::Nil)),
        }
    }
}

impl ToWeak for ast::Component<NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Text(..) => (Kind::Value, Data::Local(Type::String)),

            Self::Expression(id) => (Kind::Value, Data::Inherit(*id)),

            Self::Fragment(..) | Self::ClosedElement(..) | Self::OpenElement { .. } => {
                (Kind::Value, Data::Local(Type::Element))
            }
        }
    }
}

impl ToWeak for ast::Parameter<String, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Value,
            match self {
                Self {
                    value_type: Some(x),
                    ..
                } => Data::InheritKind(*x, Kind::Type),

                Self {
                    default_value: Some(x),
                    ..
                } => Data::Inherit(*x),

                Self { .. } => Data::Infer(Inference::Parameter),
            },
        )
    }
}

impl ToWeak for ast::TypeExpression<NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Type,
            match self {
                Self::Primitive(x) => Data::Local(match x {
                    ast::TypePrimitive::Nil => Type::Nil,
                    ast::TypePrimitive::Boolean => Type::Boolean,
                    ast::TypePrimitive::Integer => Type::Integer,
                    ast::TypePrimitive::Float => Type::Float,
                    ast::TypePrimitive::String => Type::String,
                    ast::TypePrimitive::Style => Type::Style,
                    ast::TypePrimitive::Element => Type::Element,
                }),

                Self::Group(id) => Data::Inherit(**id),

                Self::Identifier(x) => Data::Infer(Inference::Reference(x.clone())),

                Self::PropertyAccess(x, property) => {
                    Data::Infer(Inference::Property(**x, property.clone()))
                }

                Self::Function(params, x) => Data::Local(Type::Function(params.clone(), **x)),
            },
        )
    }
}

impl ToWeak for ast::Declaration<String, NodeId, NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::TypeAlias { value, .. } => (Kind::Type, Data::Inherit(*value)),

            Self::Enumerated { variants, .. } => (
                Kind::Mixed,
                Data::Local(Type::Enumerated(Enumerated::Declaration(variants.clone()))),
            ),

            Self::Constant {
                value_type, value, ..
            } => (
                Kind::Value,
                value_type
                    .map(|x| Data::InheritKind(x, Kind::Type))
                    .unwrap_or(Data::Inherit(*value)),
            ),

            Self::Function {
                parameters,
                body_type,
                body,
                ..
            } => (
                Kind::Value,
                Data::Local(Type::Function(
                    parameters.clone(),
                    body_type.unwrap_or(*body),
                )),
            ),

            Self::View { parameters, .. } => {
                (Kind::Value, Data::Local(Type::View(parameters.clone())))
            }

            Self::Module { value, .. } => (Kind::Mixed, Data::Inherit(*value)),
        }
    }
}

impl ToWeak for ast::Import {
    fn to_weak(&self) -> Weak {
        (Kind::Mixed, Data::Infer(Inference::Import(self)))
    }
}

impl ToWeak for ast::Module<NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Mixed,
            Data::Infer(Inference::Module(self.declarations.clone())),
        )
    }
}
