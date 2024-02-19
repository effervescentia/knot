use super::partial;
use crate::{data::ScopedType, error::ResolveError, strong};
use lang::{
    types::{Enumerated, RefKind, Type},
    NodeId,
};

const VALUE_KIND: RefKind = RefKind::Value;

pub fn infer<'a>(
    strong: &strong::Result,
    lhs: &NodeId,
    arguments: &[NodeId],
) -> partial::Action<'a> {
    match strong.resolve_type(lhs, &VALUE_KIND) {
        Some(Ok(x @ Type::Function(_, body))) => match strong.get_type(body, &VALUE_KIND) {
            Some(Ok(x)) => partial::Action::Infer(&Ok(x.clone())),

            Some(Err(_)) => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*body]))),

            None => partial::Action::Skip,
        },

        Some(Ok(x @ Type::Enumerated(Enumerated::Variant(parameters, instance)))) => {
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Enumerated(
                Enumerated::Instance(*instance),
            ))))
        }

        Some(Ok(_)) => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![]))),

        Some(Err(_)) => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*lhs]))),

        None => partial::Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::ScopedType, error::ResolveError, infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use lang::{
        types::{Enumerated, RefKind, Type},
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

    // #[test]
    // fn function_invalid_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Function(
    //                         vec![NodeId(0), NodeId(1)],
    //                         NodeId(2),
    //                     ))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(2)]),
    //         partial::Action::Infer(&Err(SemanticError::InvalidArguments(
    //             NodeId(3),
    //             vec![(
    //                 (ShallowType(Type::Boolean), NodeId(1)),
    //                 (ShallowType(Type::Integer), NodeId(2))
    //             )]
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(2), NodeId(1)]),
    //         partial::Action::Infer(&Err(SemanticError::InvalidArguments(
    //             NodeId(3),
    //             vec![(
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Integer), NodeId(2))
    //             )]
    //         )))
    //     );
    // }

    // #[test]
    // fn function_missing_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Function(
    //                         vec![NodeId(0), NodeId(1)],
    //                         NodeId(2),
    //                     ))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[]),
    //         partial::Action::Infer(&Err(SemanticError::MissingArguments(
    //             NodeId(3),
    //             vec![
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Boolean), NodeId(1))
    //             ]
    //         )))
    //     );
    // }

    // #[test]
    // fn function_unexpected_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Function(vec![], NodeId(2)))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(1)]),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedArguments(
    //             NodeId(3),
    //             vec![
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Boolean), NodeId(1))
    //             ]
    //         )))
    //     );
    // }

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

    // #[test]
    // fn enumerated_variant_invalid_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
    //                         vec![NodeId(0), NodeId(1)],
    //                         NodeId(2),
    //                     )))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(2)]),
    //         partial::Action::Infer(&Err(SemanticError::InvalidArguments(
    //             NodeId(3),
    //             vec![(
    //                 (ShallowType(Type::Boolean), NodeId(1)),
    //                 (ShallowType(Type::Integer), NodeId(2))
    //             )]
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(2), NodeId(1)]),
    //         partial::Action::Infer(&Err(SemanticError::InvalidArguments(
    //             NodeId(3),
    //             vec![(
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Integer), NodeId(2))
    //             )]
    //         )))
    //     );
    // }

    // #[test]
    // fn enumerated_variant_missing_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
    //                         vec![NodeId(0), NodeId(1)],
    //                         NodeId(2),
    //                     )))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[]),
    //         partial::Action::Infer(&Err(SemanticError::MissingArguments(
    //             NodeId(3),
    //             vec![
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Boolean), NodeId(1))
    //             ]
    //         )))
    //     );
    // }

    // #[test]
    // fn enumerated_variant_unexpected_arguments() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //             (
    //                 NodeId(2),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(3),
    //                 (
    //                     RefKind::Value,
    //                     Ok(ScopedType::Type(Type::Enumerated(Enumerated::Variant(
    //                         vec![],
    //                         NodeId(2),
    //                     )))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(3), &[NodeId(0), NodeId(1)]),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedArguments(
    //             NodeId(3),
    //             vec![
    //                 (ShallowType(Type::Nil), NodeId(0)),
    //                 (ShallowType(Type::Boolean), NodeId(1))
    //             ]
    //         )))
    //     );
    // }

    // #[test]
    // fn not_callable() {
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![(NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil))))],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &NodeId(0), &[NodeId(0), NodeId(1)]),
    //         partial::Action::Infer(&Err(SemanticError::NotCallable(
    //             ShallowType(Type::Nil),
    //             NodeId(0)
    //         )))
    //     );
    // }

    #[test]
    fn not_inferrable() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Err(ResolveError::NotInferrable(vec![]))),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &NodeId(0), &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
