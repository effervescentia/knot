use super::partial;
use crate::{data::ScopedType, error::SemanticError, strong};
use lang::{
    types::{Enumerated, RefKind, Type},
    NodeId,
};
use std::cmp::Ordering;

const VALUE_KIND: RefKind = RefKind::Value;

pub fn infer<'a>(
    strong: &strong::Result,
    lhs: &NodeId,
    arguments: &[NodeId],
) -> partial::Action<'a> {
    let resolve_all_types = |xs: &[NodeId]| {
        xs.iter()
            .map(|id| match strong.get_deep_type(id, &VALUE_KIND) {
                Some(Ok(x)) => Some((x.clone(), *id)),
                // FIXME: may need to forward Some(Err(_)) state to determine NotInferrable
                _ => None,
            })
            .collect::<Option<Vec<_>>>()
    };

    let resolve_arguments = |parameters: Vec<(Type<NodeId>, NodeId)>,
                             arguments: Vec<(Type<NodeId>, NodeId)>| {
        match arguments.len().cmp(&parameters.len()) {
            Ordering::Less => Err(SemanticError::MissingArguments(
                *lhs,
                parameters
                    .split_at(arguments.len())
                    .1
                    .into_iter()
                    .map(|(x, id)| (x.to_shallow(), *id))
                    .collect(),
            )),

            Ordering::Greater => Err(SemanticError::UnexpectedArguments(
                *lhs,
                arguments
                    .split_at(parameters.len())
                    .1
                    .into_iter()
                    .map(|(x, id)| (x.to_shallow(), *id))
                    .collect(),
            )),

            Ordering::Equal => {
                let mismatched = parameters
                    .into_iter()
                    .zip(arguments)
                    .filter(|((parameter_type, _), (argument_type, _))| {
                        !matches!((
                                parameter_type.preview(&kind, strong),
                                argument_type.preview(&kind, strong),
                            ), (Some(param), Some(arg)) if param == arg)
                    })
                    .collect::<Vec<_>>();

                Ok(mismatched)
            }
        }
    };

    match strong.get_deep_type(lhs, &VALUE_KIND) {
        Some(Ok(x @ Type::Function(parameters, body))) => {
            match (
                resolve_all_types(parameters),
                resolve_all_types(arguments),
                strong.get_type(body, &VALUE_KIND),
            ) {
                (Some(typed_parameters), Some(typed_arguments), Some(Ok(typed_body))) => {
                    match resolve_arguments(typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                partial::Action::Infer(&Ok(typed_body.clone()))
                            } else {
                                partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                                    *lhs, mismatched,
                                )))
                            }
                        }

                        Err(err) => partial::Action::Infer(&Err(err)),
                    }
                }

                (_, _, Some(Err(_))) => {
                    partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![*lhs])))
                }

                _ => partial::Action::Skip,
            }
        }

        Some(Ok(x @ Type::Enumerated(Enumerated::Variant(parameters, instance)))) => {
            match (resolve_all_types(parameters), resolve_all_types(arguments)) {
                (Some(typed_parameters), Some(typed_arguments)) => {
                    match resolve_arguments(typed_parameters, typed_arguments) {
                        Ok(mismatched) => {
                            if mismatched.is_empty() {
                                partial::Action::Infer(&Ok(ScopedType::Type(Type::Enumerated(
                                    Enumerated::Instance(*instance),
                                ))))
                            } else {
                                partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                                    *lhs, mismatched,
                                )))
                            }
                        }

                        Err(err) => partial::Action::Infer(&Err(err)),
                    }
                }

                _ => partial::Action::Skip,
            }
        }

        Some(Ok(x)) => {
            partial::Action::Infer(&Err(SemanticError::NotCallable(x.to_shallow(), *lhs)))
        }

        Some(Err(_)) => partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![*lhs]))),

        None => partial::Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::ScopedType, error::SemanticError, infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use lang::{
        types::{Enumerated, RefKind, ShallowType, Type},
        NodeId,
    };

    #[derive(Debug, PartialEq)]
    struct MockNode;

    #[test]
    fn skip_infer() {
        let strong = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(&strong, &NodeId(0), &[]),
            partial::Action::Skip
        );
    }

    #[test]
    fn function_result() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Function(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(2),
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Integer)))
        );
    }

    #[test]
    fn function_invalid_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Function(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(2),
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(2)]),
            partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                NodeId(3),
                vec![(
                    (ShallowType(Type::Boolean), NodeId(1)),
                    (ShallowType(Type::Integer), NodeId(2))
                )]
            )))
        );
        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(2), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                NodeId(3),
                vec![(
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Integer), NodeId(2))
                )]
            )))
        );
    }

    #[test]
    fn function_missing_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Function(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(2),
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[]),
            partial::Action::Infer(&Err(SemanticError::MissingArguments(
                NodeId(3),
                vec![
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Boolean), NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn function_unexpected_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Function(vec![], NodeId(2)))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::UnexpectedArguments(
                NodeId(3),
                vec![
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Boolean), NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn enumerated_variant_instance() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(3),
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(2), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Enumerated(
                Enumerated::Instance(NodeId(3))
            ))))
        );
    }

    #[test]
    fn enumerated_variant_invalid_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(2),
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(2)]),
            partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                NodeId(3),
                vec![(
                    (ShallowType(Type::Boolean), NodeId(1)),
                    (ShallowType(Type::Integer), NodeId(2))
                )]
            )))
        );
        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(2), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::InvalidArguments(
                NodeId(3),
                vec![(
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Integer), NodeId(2))
                )]
            )))
        );
    }

    #[test]
    fn enumerated_variant_missing_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
                            vec![NodeId(0), NodeId(1)],
                            NodeId(2),
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[]),
            partial::Action::Infer(&Err(SemanticError::MissingArguments(
                NodeId(3),
                vec![
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Boolean), NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn enumerated_variant_unexpected_arguments() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
                (
                    NodeId(2),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(3),
                    (
                        RefKind::Value,
                        Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
                            vec![],
                            NodeId(2),
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::UnexpectedArguments(
                NodeId(3),
                vec![
                    (ShallowType(Type::Nil), NodeId(0)),
                    (ShallowType(Type::Boolean), NodeId(1))
                ]
            )))
        );
    }

    #[test]
    fn not_callable() {
        let strong = strong_result_from(
            vec![],
            vec![(NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil))))],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(0), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::NotCallable(
                ShallowType(Type::Nil),
                NodeId(0)
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Err(SemanticError::NotInferrable(vec![]))),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(0), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
