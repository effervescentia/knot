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
    arguments: Vec<usize>,
    ctx: &mut StrongContext,
) -> Option<Strong> {
    let resolve_all_types = |xs: &Vec<usize>| {
        xs.iter()
            .map(|id| match ctx.get_strong(id, &RefKind::Value) {
                Some(Ok(x)) => Some((x.clone(), *id)),
                _ => None,
            })
            .collect::<Option<Vec<_>>>()
    };

    match ctx.get_strong(&lhs, &RefKind::Value) {
        Some(Ok(x @ Type::Function(parameters, result))) => {
            match (resolve_all_types(parameters), resolve_all_types(&arguments)) {
                (Some(typed_parameters), Some(typed_arguments)) => {
                    if typed_arguments.len() < typed_parameters.len() {
                        Some(Err(SemanticError::MissingArguments(
                            (x.clone(), lhs),
                            typed_parameters.split_at(typed_arguments.len()).1.to_vec(),
                        )))
                    } else if typed_arguments.len() > typed_parameters.len() {
                        Some(Err(SemanticError::UnexpectedArguments(
                            (x.clone(), lhs),
                            typed_arguments.split_at(typed_parameters.len()).1.to_vec(),
                        )))
                    } else {
                        let mismatched = typed_parameters
                            .iter()
                            .zip(typed_arguments.iter())
                            .filter(
                                |((parameter_type, parameter_id), (argument_type, argument_id))| {
                                    false
                                },
                            )
                            .collect::<Vec<_>>();

                        None
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
