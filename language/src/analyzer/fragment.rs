use super::{Type, WeakRef, WeakType};
use crate::parser::{
    declaration::Declaration,
    expression::{
        binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive, statement::Statement,
        Expression, UnaryOperator,
    },
    module::Module,
    types::type_expression::TypeExpression,
};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Fragment {
    Expression(Expression<usize, usize>),
    KSX(KSX<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize>),
    Module(Module<usize>),
}

impl Fragment {
    pub fn weak(&self) -> WeakRef {
        match self {
            Fragment::Expression(x) => match x {
                Expression::Primitive(x) => WeakRef::Value(WeakType::Strong(match x {
                    Primitive::Nil => Type::Nil,
                    Primitive::Boolean(_) => Type::Boolean,
                    Primitive::Integer(_) => Type::Integer,
                    Primitive::Float(_, _) => Type::Float,
                    Primitive::String(_) => Type::String,
                })),

                Expression::Identifier(_) => WeakRef::Value(WeakType::Any),

                Expression::Group(id) => WeakRef::Value(WeakType::Reference(**id)),

                Expression::Closure(xs) => WeakRef::Value({
                    if let Some(Statement::Effect(id) | Statement::Variable(_, id)) = xs.last() {
                        WeakType::Reference(*id)
                    } else {
                        WeakType::Strong(Type::Nil)
                    }
                }),

                Expression::UnaryOperation(op, id) => WeakRef::Value(match op {
                    UnaryOperator::Not => WeakType::Strong(Type::Boolean),

                    _ => WeakType::Reference(**id),
                }),

                Expression::BinaryOperation(op, _, _) => WeakRef::Value(match op {
                    BinaryOperator::Equal
                    | BinaryOperator::NotEqual
                    | BinaryOperator::And
                    | BinaryOperator::Or
                    | BinaryOperator::LessThan
                    | BinaryOperator::LessThanOrEqual
                    | BinaryOperator::GreaterThan
                    | BinaryOperator::GreaterThanOrEqual => WeakType::Strong(Type::Boolean),

                    BinaryOperator::Divide | BinaryOperator::Exponent => {
                        WeakType::Strong(Type::Float)
                    }

                    BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => {
                        WeakType::Any
                    }
                }),

                Expression::DotAccess(_, _) => WeakRef::Value(WeakType::Any),

                Expression::FunctionCall(_, _) => WeakRef::Value(WeakType::Any),

                Expression::Style(_) => WeakRef::Value(WeakType::Strong(Type::Style)),

                Expression::KSX(_) => WeakRef::Value(WeakType::Strong(Type::Element)),
            },

            Fragment::KSX(x) => match x {
                KSX::Text(_) => WeakRef::Value(WeakType::Strong(Type::String)),

                KSX::Inline(id) => WeakRef::Value(WeakType::Reference(*id)),

                KSX::Fragment(_) => WeakRef::Value(WeakType::Strong(Type::Element)),

                KSX::ClosedElement(_, _) => WeakRef::Value(WeakType::Strong(Type::Element)),

                KSX::OpenElement(_, _, _, _) => WeakRef::Value(WeakType::Strong(Type::Element)),
            },

            Fragment::TypeExpression(x) => WeakRef::Type(match x {
                TypeExpression::Nil => WeakType::Strong(Type::Nil),
                TypeExpression::Boolean => WeakType::Strong(Type::Boolean),
                TypeExpression::Integer => WeakType::Strong(Type::Integer),
                TypeExpression::Float => WeakType::Strong(Type::Float),
                TypeExpression::String => WeakType::Strong(Type::String),
                TypeExpression::Style => WeakType::Strong(Type::Style),
                TypeExpression::Element => WeakType::Strong(Type::Element),

                TypeExpression::Identifier(_) => WeakType::Any,

                TypeExpression::Group(id) => WeakType::Reference(**id),

                TypeExpression::DotAccess(_, _) => WeakType::Any,

                TypeExpression::Function(_, _) => WeakType::Any,
            }),

            Fragment::Declaration(x) => match x {
                Declaration::TypeAlias { value, .. } => WeakRef::Value(WeakType::Reference(*value)),

                Declaration::Enumerated { variants, .. } => {
                    WeakRef::Value(WeakType::Strong(Type::Enumerated(
                        variants
                            .into_iter()
                            .map(|(name, parameters)| {
                                (
                                    name.clone(),
                                    parameters.into_iter().map(|x| *x).collect::<Vec<_>>(),
                                )
                            })
                            .collect::<Vec<_>>(),
                    )))
                }

                Declaration::Constant {
                    value_type, value, ..
                } => WeakRef::Value(WeakType::Reference(value_type.unwrap_or(*value))),

                Declaration::Function { .. } => WeakRef::Value(WeakType::Any),

                Declaration::View { .. } => WeakRef::Value(WeakType::Any),

                Declaration::Module { .. } => WeakRef::Value(WeakType::Any),
            },

            Fragment::Module(_) => WeakRef::Value(WeakType::Any),
        }
    }
}
