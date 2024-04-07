use super::Visitor;
use crate::error::Error;
use kore::invariant;
use lang::{
    ast::{BinaryOperator, Expression, UnaryOperator},
    types::{Enumerated, ToShape, Type},
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
    _: &Visitor,
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

        Expression::FunctionCall(x, arguments) => match x.type_of() {
            Type::Function(parameters, _)
            | Type::Enumerated(Enumerated::Variant(parameters, _)) => {
                let mut errors = vec![];
                let lhs = parameters.iter().map(Some).chain(std::iter::repeat(None));
                let rhs = arguments.iter().map(Some).chain(std::iter::repeat(None));

                // TODO: handle case where optional parameters appear before required parameters
                for pair in lhs.zip(rhs) {
                    match pair {
                        // TODO: should this use a more nuanced approach for comparing types?
                        // how will this handle enumerators for example?
                        (Some(parameter), Some(argument))
                            if parameter.to_shape() == argument.type_of().to_shape() => {}

                        (Some(parameter), Some(argument)) => {
                            errors.push(Error::ArgumentRejected(*parameter.id(), *argument.id()));
                        }

                        (None, Some(argument)) => {
                            errors.push(Error::UnexpectedArgument(*argument.id()));
                        }

                        // TODO: this doesn't take into account default values
                        // need to bake it into the type definition
                        (Some(parameter), None) => {
                            errors.push(Error::MissingArgument(*parameter.id()));
                        }

                        // this case is not possible
                        (None, None) => (),
                    }
                }

                (!errors.is_empty()).then_some(errors)
            }

            _ => invariant!("this should have been raised as NotCallable during inference"),
        },

        Expression::Style(_) => None,

        Expression::Component(_) => None,
    }
}
