use crate::{
    context::StrongContext,
    infer::strong::{SemanticError, Strong},
    RefKind, Type,
};

pub fn infer(lhs: usize, arguments: &Vec<usize>, ctx: &StrongContext) -> Option<Strong> {
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
                            (Some(param), Some(arg)) if param == arg => false,
                            _ => true,
                        }
                    })
                    .collect::<Vec<_>>();

                Ok(mismatched)
            }
        };

    match ctx.get_strong(&lhs, &kind)? {
        Ok(x @ Type::Function(parameters, result)) => {
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
                                Some(Err(SemanticError::InvalidArguments(
                                    (x.clone(), lhs),
                                    mismatched,
                                )))
                            }
                        }

                        Err(err) => Some(Err(err)),
                    }
                }

                (_, _, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),

                _ => None,
            }
        }

        Ok(x @ Type::EnumeratedVariant(parameters, result)) => {
            match (resolve_all_types(parameters), resolve_all_types(arguments)) {
                (Some(typed_parameters), Some(typed_arguments)) => {
                    match resolve_arguments(x.clone(), typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                Some(Ok(Type::EnumeratedInstance(*result)))
                            } else {
                                Some(Err(SemanticError::InvalidArguments(
                                    (x.clone(), lhs),
                                    mismatched,
                                )))
                            }
                        }

                        Err(err) => Some(Err(err)),
                    }
                }

                _ => None,
            }
        }

        Ok(x) => Some(Err(SemanticError::NotCallable(x.clone(), lhs))),

        Err(_) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::StrongContext,
        infer::strong::{SemanticError, Strong},
        test::fixture::strong_ctx_from,
        types::Type,
        RefKind,
    };

    fn infer(lhs: usize, arguments: Vec<usize>, ctx: &StrongContext) -> Option<Strong> {
        super::infer(lhs, &arguments, ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(infer(0, vec![], &ctx), None);
    }

    #[test]
    fn function_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(Type::Function(vec![0, 1], 2)))),
            ],
            vec![],
        );

        assert_eq!(infer(3, vec![0, 1], &ctx), Some(Ok(Type::Integer)));
    }

    #[test]
    fn function_invalid_arguments() {
        let func_type = || Type::Function(vec![0, 1], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![0, 2], &ctx),
            Some(Err(SemanticError::InvalidArguments(
                (func_type(), 3),
                vec![((Type::Boolean, 1), (Type::Integer, 2))]
            )))
        );
        assert_eq!(
            infer(3, vec![2, 1], &ctx),
            Some(Err(SemanticError::InvalidArguments(
                (func_type(), 3),
                vec![((Type::Nil, 0), (Type::Integer, 2))]
            )))
        );
    }

    #[test]
    fn function_missing_arguments() {
        let func_type = || Type::Function(vec![0, 1], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![], &ctx),
            Some(Err(SemanticError::MissingArguments(
                (func_type(), 3),
                vec![(Type::Nil, 0), (Type::Boolean, 1)]
            )))
        );
    }

    #[test]
    fn function_unexpected_arguments() {
        let func_type = || Type::Function(vec![], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![0, 1], &ctx),
            Some(Err(SemanticError::UnexpectedArguments(
                (func_type(), 3),
                vec![(Type::Nil, 0), (Type::Boolean, 1)]
            )))
        );
    }

    #[test]
    fn enumerated_variant_instance() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (
                    2,
                    (RefKind::Value, Ok(Type::EnumeratedVariant(vec![0, 1], 3))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            infer(2, vec![0, 1], &ctx),
            Some(Ok(Type::EnumeratedInstance(3)))
        );
    }

    #[test]
    fn enumerated_variant_invalid_arguments() {
        let variant_type = || Type::EnumeratedVariant(vec![0, 1], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(variant_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![0, 2], &ctx),
            Some(Err(SemanticError::InvalidArguments(
                (variant_type(), 3),
                vec![((Type::Boolean, 1), (Type::Integer, 2))]
            )))
        );
        assert_eq!(
            infer(3, vec![2, 1], &ctx),
            Some(Err(SemanticError::InvalidArguments(
                (variant_type(), 3),
                vec![((Type::Nil, 0), (Type::Integer, 2))]
            )))
        );
    }

    #[test]
    fn enumerated_variant_missing_arguments() {
        let variant_type = || Type::EnumeratedVariant(vec![0, 1], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(variant_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![], &ctx),
            Some(Err(SemanticError::MissingArguments(
                (variant_type(), 3),
                vec![(Type::Nil, 0), (Type::Boolean, 1)]
            )))
        );
    }

    #[test]
    fn enumerated_variant_unexpected_arguments() {
        let enum_type = || Type::EnumeratedVariant(vec![], 2);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
                (2, (RefKind::Value, Ok(Type::Integer))),
                (3, (RefKind::Value, Ok(enum_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(3, vec![0, 1], &ctx),
            Some(Err(SemanticError::UnexpectedArguments(
                (enum_type(), 3),
                vec![(Type::Nil, 0), (Type::Boolean, 1)]
            )))
        );
    }

    #[test]
    fn not_callable() {
        let ctx = strong_ctx_from(vec![], vec![(0, (RefKind::Value, Ok(Type::Nil)))], vec![]);

        assert_eq!(
            infer(0, vec![0, 1], &ctx),
            Some(Err(SemanticError::NotCallable(Type::Nil, 0)))
        );
    }

    #[test]
    fn not_inferrable() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(
                0,
                (RefKind::Value, Err(SemanticError::NotInferrable(vec![]))),
            )],
            vec![],
        );

        assert_eq!(
            infer(0, vec![0, 1], &ctx),
            Some(Err(SemanticError::NotInferrable(vec![0])))
        );
    }
}
