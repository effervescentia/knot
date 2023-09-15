use super::{RefKind, Type, WeakRef, WeakType};
use crate::parser::{
    declaration::{storage::Storage, Declaration},
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
    Expression(Expression<usize, usize, usize>),
    Statement(Statement<usize>),
    KSX(KSX<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize>),
    Module(Module<usize>),
}

impl Fragment {
    pub fn to_weak(&self) -> WeakRef {
        match self {
            Fragment::Expression(x) => match x {
                Expression::Primitive(x) => (
                    RefKind::Value,
                    WeakType::Strong(match x {
                        Primitive::Nil => Type::Nil,
                        Primitive::Boolean(..) => Type::Boolean,
                        Primitive::Integer(..) => Type::Integer,
                        Primitive::Float(..) => Type::Float,
                        Primitive::String(..) => Type::String,
                    }),
                ),

                Expression::Identifier(..) => (RefKind::Value, WeakType::Any),

                Expression::Group(id) => (RefKind::Value, WeakType::Reference(**id)),

                Expression::Closure(xs) => (RefKind::Value, {
                    if let Some(id) = xs.last() {
                        WeakType::Reference(*id)
                    } else {
                        WeakType::Strong(Type::Nil)
                    }
                }),

                Expression::UnaryOperation(op, id) => (
                    RefKind::Value,
                    match op {
                        UnaryOperator::Not => WeakType::Strong(Type::Boolean),

                        _ => WeakType::Reference(**id),
                    },
                ),

                Expression::BinaryOperation(op, ..) => (
                    RefKind::Value,
                    match op {
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

                        BinaryOperator::Add
                        | BinaryOperator::Subtract
                        | BinaryOperator::Multiply => WeakType::Any,
                    },
                ),

                Expression::DotAccess(..) => (RefKind::Value, WeakType::Any),

                Expression::FunctionCall(..) => (RefKind::Value, WeakType::Any),

                Expression::Style(..) => (RefKind::Value, WeakType::Strong(Type::Style)),

                Expression::KSX(..) => (RefKind::Value, WeakType::Strong(Type::Element)),
            },

            Fragment::Statement(x) => match x {
                Statement::Effect(id) => (RefKind::Value, WeakType::Reference(*id)),

                Statement::Variable(..) => (RefKind::Value, WeakType::Strong(Type::Nil)),
            },

            Fragment::KSX(x) => match x {
                KSX::Text(..) => (RefKind::Value, WeakType::Strong(Type::String)),

                KSX::Inline(id) => (RefKind::Value, WeakType::Reference(*id)),

                KSX::Fragment(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

                KSX::ClosedElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

                KSX::OpenElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),
            },

            Fragment::TypeExpression(x) => (
                RefKind::Type,
                match x {
                    TypeExpression::Nil => WeakType::Strong(Type::Nil),
                    TypeExpression::Boolean => WeakType::Strong(Type::Boolean),
                    TypeExpression::Integer => WeakType::Strong(Type::Integer),
                    TypeExpression::Float => WeakType::Strong(Type::Float),
                    TypeExpression::String => WeakType::Strong(Type::String),
                    TypeExpression::Style => WeakType::Strong(Type::Style),
                    TypeExpression::Element => WeakType::Strong(Type::Element),

                    TypeExpression::Identifier(..) => WeakType::Any,

                    TypeExpression::Group(id) => WeakType::Reference(**id),

                    TypeExpression::DotAccess(..) => WeakType::Any,

                    TypeExpression::Function(..) => WeakType::Any,
                },
            ),

            Fragment::Declaration(x) => match x {
                Declaration::TypeAlias { value, .. } => {
                    (RefKind::Type, WeakType::Reference(*value))
                }

                Declaration::Enumerated { variants, .. } => (
                    RefKind::Value,
                    WeakType::Strong(Type::Enumerated(
                        variants
                            .into_iter()
                            .map(|(name, parameters)| {
                                (
                                    name.clone(),
                                    parameters.into_iter().map(|x| *x).collect::<Vec<_>>(),
                                )
                            })
                            .collect::<Vec<_>>(),
                    )),
                ),

                Declaration::Constant {
                    value_type, value, ..
                } => (
                    RefKind::Value,
                    WeakType::Reference(value_type.unwrap_or(*value)),
                ),

                Declaration::Function { .. } => (RefKind::Value, WeakType::Any),

                Declaration::View { .. } => (RefKind::Value, WeakType::Any),

                Declaration::Module { .. } => (RefKind::Value, WeakType::Any),
            },

            Fragment::Module(..) => (RefKind::Value, WeakType::Any),
        }
    }

    pub fn to_binding(&self) -> Option<String> {
        match self {
            Fragment::Statement(Statement::Variable(name, ..))
            | Fragment::Declaration(
                Declaration::TypeAlias {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Enumerated {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Constant {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Function {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::View {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Module {
                    name: Storage(_, name),
                    ..
                },
            ) => Some(name.clone()),

            _ => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use crate::{
        analyzer::{RefKind, Type, WeakType},
        parser::expression::{primitive::Primitive, statement::Statement, Expression},
        test::fixture as f,
    };

    #[test]
    fn weak_primitive() {
        assert_eq!(
            Fragment::Expression(Expression::Primitive(Primitive::Nil)).to_weak(),
            (RefKind::Value, WeakType::Strong(Type::Nil))
        );
    }

    #[test]
    fn binding_variable() {
        assert_eq!(
            Fragment::Statement(Statement::Variable(String::from("foo"), 0)).to_binding(),
            Some(String::from("foo"))
        );
    }

    #[test]
    fn binding_type_alias() {
        assert_eq!(
            Fragment::Declaration(f::a::type_("Foo", 0)).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_enumerated() {
        assert_eq!(
            Fragment::Declaration(f::a::enum_("Foo", vec![])).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_constant() {
        assert_eq!(
            Fragment::Declaration(f::a::const_("FOO", None, 0)).to_binding(),
            Some(String::from("FOO"))
        );
    }

    #[test]
    fn binding_function() {
        assert_eq!(
            Fragment::Declaration(f::a::func_("foo", vec![], None, 0)).to_binding(),
            Some(String::from("foo"))
        );
    }

    #[test]
    fn binding_view() {
        assert_eq!(
            Fragment::Declaration(f::a::view("Foo", vec![], 0)).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_module() {
        assert_eq!(
            Fragment::Declaration(f::a::mod_("foo", 0)).to_binding(),
            Some(String::from("foo"))
        );
    }
}
