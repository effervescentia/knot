use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Type, WeakRef, WeakType},
    parser::expression::{
        binary_operation::BinaryOperator, primitive::Primitive, Expression, UnaryOperator,
    },
};

impl ToWeak for Expression<usize, usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
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

                    BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => {
                        WeakType::Any
                    }
                },
            ),

            Expression::DotAccess(..) => (RefKind::Value, WeakType::Any),

            Expression::FunctionCall(..) => (RefKind::Value, WeakType::Any),

            Expression::Style(..) => (RefKind::Value, WeakType::Strong(Type::Style)),

            Expression::KSX(..) => (RefKind::Value, WeakType::Strong(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        parser::{
            expression::{
                binary_operation::BinaryOperator, primitive::Primitive, Expression, UnaryOperator,
            },
            ksx::KSX,
            statement::Statement,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;
}
