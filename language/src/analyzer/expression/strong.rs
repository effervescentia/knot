use crate::{
    analyzer::{
        context::{NodeContext, StrongContext},
        infer::strong::{ExpectedType, SemanticError, Strong, ToStrong},
        RefKind, Type,
    },
    ast::{expression::ExpressionNode, operator::BinaryOperator},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<'a, T> ToStrong<'a, ExpressionNode<T, Strong>> for ExpressionNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &'a StrongContext) -> ExpressionNode<T, Strong> {
        ExpressionNode(Node(
            self.node().value().map(
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
                &mut |x| x.to_strong(ctx),
            ),
            self.node().range().clone(),
            ctx.resolve(self.node().id()).clone(),
        ))
    }
}

pub fn infer_binary_operation(
    op: &BinaryOperator,
    lhs: usize,
    rhs: usize,
    ctx: &mut StrongContext,
) -> Option<Strong> {
    match op {
        BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => match (
            ctx.get_strong(&lhs, &RefKind::Value),
            ctx.get_strong(&rhs, &RefKind::Value),
        ) {
            (Some(Ok(Type::Integer)), Some(Ok(Type::Integer))) => Some(Ok(Type::Integer)),

            (Some(Ok(Type::Integer | Type::Float)), Some(Ok(Type::Integer | Type::Float))) => {
                Some(Ok(Type::Float))
            }

            (Some(Ok(Type::Integer | Type::Float)), Some(Ok(rhs_type))) => {
                Some(Err(SemanticError::UnexpectedShape(
                    (rhs_type.clone(), rhs),
                    ExpectedType::Union(vec![Type::Integer, Type::Float]),
                )))
            }

            (Some(Ok(lhs_type)), _) => Some(Err(SemanticError::UnexpectedShape(
                (lhs_type.clone(), lhs),
                ExpectedType::Union(vec![Type::Integer, Type::Float]),
            ))),

            (Some(Err(_)), Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![lhs, rhs]))),

            (Some(Err(_)), _) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),

            (_, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![rhs]))),

            (_, None) | (None, _) => None,
        },

        _ => {
            unreachable!("all other binary operation types should be inferred during weak analysis")
        }
    }
}

pub fn infer_dot_access(lhs: usize, rhs: String, ctx: &mut StrongContext) -> Option<Strong> {
    match ctx.get_strong(&lhs, &RefKind::Value) {
        Some(Ok(x @ Type::Module(declarations))) => {
            match declarations.iter().find(|(name, ..)| *name == rhs) {
                Some((_, RefKind::Value | RefKind::Mixed, declaration_id)) => ctx
                    .get_strong(&declaration_id, &RefKind::Value)
                    .map(|x| x.clone()),

                Some((_, RefKind::Type, _)) => Some(Err(SemanticError::IllegalTypeAccess(
                    (x.clone(), lhs),
                    rhs.clone(),
                ))),

                None => Some(Err(SemanticError::DeclarationNotFound(
                    (x.clone(), lhs),
                    rhs.clone(),
                ))),
            }
        }

        Some(Ok(x @ Type::Enumerated(variants))) => {
            match variants.iter().find(|(name, _)| *name == rhs) {
                Some((_, parameters)) => Some(Ok(Type::EnumeratedVariant(parameters.clone(), lhs))),

                None => Some(Err(SemanticError::VariantNotFound(
                    (x.clone(), lhs),
                    rhs.clone(),
                ))),
            }
        }

        Some(Ok(x)) => Some(Err(SemanticError::NotIndexable(
            (x.clone(), lhs),
            rhs.clone(),
        ))),

        Some(Err(_)) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),

        None => None,
    }
}

pub fn infer_function_call(
    lhs: usize,
    arguments: &Vec<usize>,
    ctx: &mut StrongContext,
) -> Option<Strong> {
    let kind = RefKind::Value;

    let resolve_all_types = |xs: &Vec<usize>| {
        xs.iter()
            .map(|id| match ctx.get_strong(id, &kind) {
                Some(Ok(x)) => Some((x.clone(), *id)),
                // FIXME: may need to forward Some(Err(_)) state to determine NotInferrable
                _ => None,
            })
            .collect::<Option<Vec<_>>>()
    };

    let resolve_arguments =
        |typ, parameters: Vec<(Type<usize>, usize)>, arguments: Vec<(Type<usize>, usize)>| {
            if arguments.len() < parameters.len() {
                Err(SemanticError::MissingArguments(
                    (typ, lhs),
                    parameters.split_at(arguments.len()).1.to_vec(),
                ))
            } else if arguments.len() > parameters.len() {
                Err(SemanticError::UnexpectedArguments(
                    (typ, lhs),
                    arguments.split_at(parameters.len()).1.to_vec(),
                ))
            } else {
                let mismatched = parameters
                    .into_iter()
                    .zip(arguments.into_iter())
                    .filter(|((parameter_type, _), (argument_type, _))| {
                        match (
                            parameter_type.preview(&kind, ctx),
                            argument_type.preview(&kind, ctx),
                        ) {
                            (Some(param), Some(arg)) if param == arg => true,
                            _ => false,
                        }
                    })
                    .collect::<Vec<_>>();

                Ok(mismatched)
            }
        };

    match ctx.get_strong(&lhs, &kind) {
        Some(Ok(x @ Type::Function(parameters, result))) => {
            match (
                resolve_all_types(parameters),
                resolve_all_types(arguments),
                ctx.get_strong(result, &kind),
            ) {
                (Some(typed_parameters), Some(typed_arguments), Some(Ok(typed_result))) => {
                    match resolve_arguments(x.clone(), typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                Some(Ok(typed_result.clone()))
                            } else {
                                Some(Err(SemanticError::InvalidArguments(mismatched)))
                            }
                        }

                        Err(err) => Some(Err(err)),
                    }
                }

                (_, _, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),

                _ => None,
            }
        }

        Some(Ok(x @ Type::EnumeratedVariant(parameters, result))) => {
            match (resolve_all_types(parameters), resolve_all_types(arguments)) {
                (Some(typed_parameters), Some(typed_arguments)) => {
                    match resolve_arguments(x.clone(), typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                Some(Ok(Type::EnumeratedInstance(*result)))
                            } else {
                                Some(Err(SemanticError::InvalidArguments(mismatched)))
                            }
                        }

                        Err(err) => Some(Err(err)),
                    }
                }

                _ => None,
            }
        }

        Some(Ok(x)) => Some(Err(SemanticError::NotCallable(x.clone(), lhs))),

        Some(Err(_)) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),

        None => None,
    }
}
