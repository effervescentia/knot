use std::collections::HashMap;

use super::Visitor;
use crate::error::Error;
use kore::{internal, invariant};
use lang::{
    ast::{BinaryOperator, Expression, UnaryOperator},
    types::{self, Enumerated, ToShape, Type},
    walk::{CommonVisitor, ProgramVisitor},
    Identify, TypeOf,
};

pub fn analyze(
    x: &Expression<
        <Visitor as ProgramVisitor>::Expression,
        <Visitor as ProgramVisitor>::Statement,
        <Visitor as ProgramVisitor>::Component,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    visitor: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        Expression::Primitive(_) => None,

        Expression::Identifier(_) => None,

        Expression::Group(_) => None,

        Expression::Closure(_) => None,

        Expression::UnaryOperation(op, x) => match (op, x.type_of()) {
            (UnaryOperator::Not, Type::Boolean) => None,

            (UnaryOperator::Absolute | UnaryOperator::Negate, Type::Integer | Type::Float) => None,

            _ => Some(vec![Error::UnaryOperationNotSupported(
                *op,
                (*x.id(), Some(x.type_of().to_shape())),
            )]),
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

                (_, lhs_type, rhs_type) => Some(vec![Error::BinaryOperationNotSupported(
                    *op,
                    (*lhs.id(), Some(lhs_type.to_shape())),
                    (*rhs.id(), Some(rhs_type.to_shape())),
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
                        (Some(parameter), Some(argument)) => {
                            let parameter_shape = parameter.to_shape();
                            let argument_shape = argument.type_of().to_shape();

                            if parameter_shape != argument_shape {
                                errors.push(Error::ArgumentRejected(
                                    (*parameter.id(), parameter_shape),
                                    (*argument.id(), argument_shape),
                                ));
                            }
                        }

                        (None, Some(argument)) => {
                            errors
                                .push(Error::UnexpectedArgument(*argument.id(), parameters.len()));
                        }

                        // TODO: this doesn't take into account default values
                        // need to bake it into the type definition
                        (Some(parameter), None) => {
                            errors.push(Error::MissingArgument(
                                *parameter.id(),
                                parameter.to_shape(),
                            ));
                        }

                        (None, None) => break,
                    }
                }

                (!errors.is_empty()).then_some(errors)
            }

            _ => invariant!("this should have been raised as NotCallable during inference"),
        },

        Expression::Style(rules) => {
            let mut errors = vec![];

            let style_namespace_id = visitor
                .context
                .ambient
                .get(&internal::AmbientScope::Style)?;
            let style_module = visitor.context.modules.by_key.get(style_namespace_id)?;
            let style_rules =
                style_module
                    .2
                    .get(&style_module.0)
                    .and_then(|x| match &x.to_shape().0 {
                        types::Type::Module(xs) => Some(
                            xs.iter()
                                .filter(|(_, kind, _)| (kind == &types::Kind::Value))
                                .filter_map(|(name, _, type_)| match &type_.as_ref().0 {
                                    types::Type::Function(parameters, _)
                                        if parameters.len() == 1 =>
                                    {
                                        Some((
                                            name.to_owned(),
                                            parameters.first()?.as_ref().clone(),
                                        ))
                                    }
                                    _ => None,
                                })
                                .collect::<HashMap<_, _>>(),
                        ),

                        _ => None,
                    })?;

            for (key, value) in rules {
                if let Some(expected_type) = style_rules.get(key) {
                    let actual_type = value.type_of().to_shape();

                    if &actual_type != expected_type && actual_type.0 != types::Type::String {
                        errors.push(Error::StyleRuleRejected(
                            key.clone(),
                            expected_type.clone(),
                            actual_type,
                        ));
                    }
                } else {
                    errors.push(Error::StyleRuleNotFound(key.clone()));
                }
            }

            (!errors.is_empty()).then_some(errors)
        }

        Expression::Component(_) => None,
    }
}
