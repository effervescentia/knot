use crate::{
    infer::strong::SemanticError,
    strong::{Strong, StrongResult},
};
use lang::{types, NodeId};
use std::cmp::Ordering;

pub fn infer(lhs: &NodeId, arguments: &[NodeId], result: &StrongResult) -> Option<Strong> {
    let kind = types::RefKind::Value;

    let resolve_all_types = |xs: &[NodeId]| {
        xs.iter()
            .map(|id| match result.as_strong(id, &kind) {
                Some(Ok(x)) => Some((x.clone(), *id)),
                // FIXME: may need to forward Some(Err(_)) state to determine NotInferrable
                _ => None,
            })
            .collect::<Option<Vec<_>>>()
    };

    let resolve_arguments =
        |typ,
         parameters: Vec<(types::Type<NodeId>, NodeId)>,
         arguments: Vec<(types::Type<NodeId>, NodeId)>| {
            match arguments.len().cmp(&parameters.len()) {
                Ordering::Less => Err(SemanticError::MissingArguments(
                    (typ, *lhs),
                    parameters.split_at(arguments.len()).1.to_vec(),
                )),

                Ordering::Greater => Err(SemanticError::UnexpectedArguments(
                    (typ, *lhs),
                    arguments.split_at(parameters.len()).1.to_vec(),
                )),

                Ordering::Equal => {
                    let mismatched = parameters
                        .into_iter()
                        .zip(arguments)
                        .filter(|((parameter_type, _), (argument_type, _))| {
                            !matches!((
                                parameter_type.preview(&kind, result),
                                argument_type.preview(&kind, result),
                            ), (Some(param), Some(arg)) if param == arg)
                        })
                        .collect::<Vec<_>>();

                    Ok(mismatched)
                }
            }
        };

    match result.as_strong(lhs, &kind)? {
        Ok(x @ types::Type::Function(parameters, result)) => {
            match (
                resolve_all_types(parameters),
                resolve_all_types(arguments),
                result.as_strong(result, &kind),
            ) {
                (Some(typed_parameters), Some(typed_arguments), Some(Ok(typed_result))) => {
                    match resolve_arguments(x.clone(), typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                Some(Ok(typed_result.clone()))
                            } else {
                                Some(Err(SemanticError::InvalidArguments(
                                    (x.clone(), *lhs),
                                    mismatched,
                                )))
                            }
                        }

                        Err(err) => Some(Err(err)),
                    }
                }

                (_, _, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![*lhs]))),

                _ => None,
            }
        }

        Ok(x @ types::Type::Enumerated(types::Enumerated::Variant(parameters, result))) => {
            match (resolve_all_types(parameters), resolve_all_types(arguments)) {
                (Some(typed_parameters), Some(typed_arguments)) => {
                    match resolve_arguments(x.clone(), typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                Some(Ok(types::Type::Enumerated(types::Enumerated::Instance(
                                    *result,
                                ))))
                            } else {
                                Some(Err(SemanticError::InvalidArguments(
                                    (x.clone(), *lhs),
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

        Ok(x) => Some(Err(SemanticError::NotCallable(x.clone(), *lhs))),

        Err(_) => Some(Err(SemanticError::NotInferrable(vec![*lhs]))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{infer::strong::SemanticError, test::fixture::strong_result_from};
    use lang::{types, NodeId};

    #[test]
    fn none_result() {
        let result = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(super::infer(&NodeId(0), &[], &result), None);
    }

    #[test]
    fn function_result() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (
                    NodeId(3),
                    (
                        types::RefKind::Value,
                        Ok(types::Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(0), NodeId(1)], &result),
            Some(Ok(types::Type::Integer))
        );
    }

    #[test]
    fn function_invalid_arguments() {
        let func_type = || types::Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2));
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(0), NodeId(2)], &result),
            Some(Err(SemanticError::InvalidArguments(
                (func_type(), NodeId(3)),
                vec![(
                    (types::Type::Boolean, NodeId(1)),
                    (types::Type::Integer, NodeId(2))
                )]
            )))
        );
        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(2), NodeId(1)], &result),
            Some(Err(SemanticError::InvalidArguments(
                (func_type(), NodeId(3)),
                vec![(
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Integer, NodeId(2))
                )]
            )))
        );
    }

    #[test]
    fn function_missing_arguments() {
        let func_type = || types::Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2));
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[], &result),
            Some(Err(SemanticError::MissingArguments(
                (func_type(), NodeId(3)),
                vec![
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Boolean, NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn function_unexpected_arguments() {
        let func_type = || types::Type::Function(vec![], NodeId(2));
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(func_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(0), NodeId(1)], &result),
            Some(Err(SemanticError::UnexpectedArguments(
                (func_type(), NodeId(3)),
                vec![
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Boolean, NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn enumerated_variant_instance() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (
                    NodeId(2),
                    (
                        types::RefKind::Value,
                        Ok(types::Type::Enumerated(types::Enumerated::Variant(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(3),
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(2), &[NodeId(0), NodeId(1)], &result),
            Some(Ok(types::Type::Enumerated(types::Enumerated::Instance(
                NodeId(3)
            ))))
        );
    }

    #[test]
    fn enumerated_variant_invalid_arguments() {
        let variant_type = || {
            types::Type::Enumerated(types::Enumerated::Variant(
                vec![NodeId(0), NodeId(1)],
                NodeId(2),
            ))
        };
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(variant_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(0), NodeId(2)], &result),
            Some(Err(SemanticError::InvalidArguments(
                (variant_type(), NodeId(3)),
                vec![(
                    (types::Type::Boolean, NodeId(1)),
                    (types::Type::Integer, NodeId(2))
                )]
            )))
        );
        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(2), NodeId(1)], &result),
            Some(Err(SemanticError::InvalidArguments(
                (variant_type(), NodeId(3)),
                vec![(
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Integer, NodeId(2))
                )]
            )))
        );
    }

    #[test]
    fn enumerated_variant_missing_arguments() {
        let variant_type = || {
            types::Type::Enumerated(types::Enumerated::Variant(
                vec![NodeId(0), NodeId(1)],
                NodeId(2),
            ))
        };
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(variant_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[], &result),
            Some(Err(SemanticError::MissingArguments(
                (variant_type(), NodeId(3)),
                vec![
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Boolean, NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn enumerated_variant_unexpected_arguments() {
        let enum_type = || types::Type::Enumerated(types::Enumerated::Variant(vec![], NodeId(2)));
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(3), (types::RefKind::Value, Ok(enum_type()))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(3), &[NodeId(0), NodeId(1)], &result),
            Some(Err(SemanticError::UnexpectedArguments(
                (enum_type(), NodeId(3)),
                vec![
                    (types::Type::Nil, NodeId(0)),
                    (types::Type::Boolean, NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn not_callable() {
        let result = strong_result_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil)))],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(0), &[NodeId(0), NodeId(1)], &result),
            Some(Err(SemanticError::NotCallable(types::Type::Nil, NodeId(0))))
        );
    }

    #[test]
    fn not_inferrable() {
        let result = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (
                    types::RefKind::Value,
                    Err(SemanticError::NotInferrable(vec![])),
                ),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(0), &[NodeId(0), NodeId(1)], &result),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
