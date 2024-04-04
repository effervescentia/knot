use super::Visitor;
use crate::error::Error;
use lang::{
    ast::{BinaryOperator, Expression, UnaryOperator},
    types::{ToShape, Type},
    walk::Visit,
    Identify, TypeOf,
};

pub fn analyze(
    x: &Expression<
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::Statement,
        <Visitor as Visit>::Component,
    >,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    match x {
        Expression::Primitive(_) => None,

        Expression::Identifier(_) => None,

        Expression::Group(_) => None,

        Expression::Closure(_) => None,

        Expression::UnaryOperation(op, x) => match (op, x.type_of()) {
            (UnaryOperator::Not, Type::Boolean) => None,

            (UnaryOperator::Absolute | UnaryOperator::Negate, Type::Integer | Type::Float) => None,

            _ => Some(vec![Error::UnaryOperationNotSupported(*op, *x.id())]),
        },

        Expression::BinaryOperation(op, lhs, rhs) => {
            match (op, lhs.type_of(), rhs.type_of()) {
                (BinaryOperator::And | BinaryOperator::Or, Type::Boolean, Type::Boolean) => None,

                (
                    BinaryOperator::LessThan
                    | BinaryOperator::LessThanOrEqual
                    | BinaryOperator::GreaterThan
                    | BinaryOperator::GreaterThanOrEqual
                    | BinaryOperator::Add
                    | BinaryOperator::Subtract
                    | BinaryOperator::Multiply
                    | BinaryOperator::Divide
                    | BinaryOperator::Exponent,
                    Type::Integer | Type::Float,
                    Type::Integer | Type::Float,
                ) => None,

                // TODO: should this use a more nuanced approach for comparing types?
                // how will this handle enumerators for example?
                (BinaryOperator::Equal | BinaryOperator::NotEqual, l, r)
                    if l.to_shape() == r.to_shape() =>
                {
                    None
                }

                _ => Some(vec![Error::BinaryOperationNotSupported(
                    *op,
                    *lhs.id(),
                    *rhs.id(),
                )]),
            }
        }

        Expression::PropertyAccess(..) => None,

        Expression::FunctionCall(x, arguments) => None,

        Expression::Style(_) => None,

        Expression::Component(_) => None,
    }
}
