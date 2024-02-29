use super::partial;
use crate::{data::NodeKind, error::ResolveError, strong};
use lang::{
    types::{Enumerated, ReferenceType, Type},
    NodeId,
};

pub fn infer<'a, Node>(
    state: &strong::State,
    node: &Node,
    lhs: &NodeId,
    property: &str,
) -> partial::Action<'a>
where
    Node: NodeKind,
{
    match state.resolve_type(lhs, node.kind()) {
        Some(Ok(ReferenceType(x @ Type::Module(declarations)))) => {
            match declarations.iter().find(|(name, ..)| name == property) {
                Some((_, declaration_kind, declaration_type))
                    if node.kind().can_accept(declaration_kind) =>
                {
                    partial::Action::Infer(&Ok(declaration_type))
                }

                Some(_) | None => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![]))),
            }
        }

        Some(Ok(x @ ReferenceType(Type::Enumerated(Enumerated::Declaration(variants))))) => {
            match variants.iter().find(|(name, _)| name == property) {
                Some((_, parameters)) => partial::Action::Infer(&Ok(&ReferenceType(
                    Type::Enumerated(Enumerated::Variant(parameters.clone(), x)),
                ))),

                None => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![]))),
            }
        }

        Some(_) => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*lhs]))),

        None => partial::Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::NodeKind, error::ResolveError, infer::strong::partial,
        test::fixture::strong_state_from,
    };
    use kore::str;
    use lang::{
        types::{Enumerated, RefKind, ReferenceType, Type},
        NodeId,
    };

    #[derive(Debug, PartialEq)]
    struct MockNode(RefKind);

    impl NodeKind for MockNode {
        fn kind(&self) -> &RefKind {
            &self.0
        }
    }

    #[test]
    fn skip_infer() {
        let state = strong_state_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(&state, &MockNode(RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Skip
        );
    }

    #[test]
    fn module_entry_result() {
        let state = strong_state_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(1), (RefKind::Type, Ok(&ReferenceType(Type::Nil)))),
                (
                    NodeId(2),
                    (
                        RefKind::Mixed,
                        Ok(&ReferenceType(Type::Module(vec![
                            (str!("foo"), RefKind::Value, &ReferenceType(Type::Nil)),
                            (str!("bar"), RefKind::Type, &ReferenceType(Type::Nil)),
                        ]))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&state, &MockNode(RefKind::Value), &NodeId(2), "foo"),
            partial::Action::Infer(&Ok(&ReferenceType(Type::Nil)))
        );
        assert_eq!(
            super::infer(&state, &MockNode(RefKind::Type), &NodeId(2), "bar"),
            partial::Action::Infer(&Ok(&ReferenceType(Type::Nil)))
        );
    }

    // #[test]
    // fn module_illegal_type_access() {
    //     let module_type = || {
    //         Type::Module(vec![
    //             (str!("foo"), RefKind::Value, NodeId(0)),
    //             (str!("bar"), RefKind::Type, NodeId(1)),
    //         ])
    //     };
    //     let state = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (NodeId(1), (RefKind::Type, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(2),
    //                 (
    //                     RefKind::Mixed,
    //                     Ok(ScopedType::Type(Type::Module(vec![
    //                         (str!("foo"), RefKind::Value, NodeId(0)),
    //                         (str!("bar"), RefKind::Type, NodeId(1)),
    //                     ]))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Type), &NodeId(2), "foo"),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedKind(
    //             (RefKind::Value, NodeId(2)),
    //             RefKind::Type
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Value), &NodeId(2), "bar"),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedKind(
    //             (RefKind::Type, NodeId(2)),
    //             RefKind::Value
    //         )))
    //     );
    // }

    // #[test]
    // fn module_declaration_not_found() {
    //     let state = strong_result_from(
    //         vec![],
    //         vec![
    //             (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
    //             (
    //                 NodeId(1),
    //                 (
    //                     RefKind::Mixed,
    //                     Ok(ScopedType::Type(Type::Module(vec![(
    //                         str!("foo"),
    //                         RefKind::Value,
    //                         NodeId(0),
    //                     )]))),
    //                 ),
    //             ),
    //         ],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Value), &NodeId(1), "bar"),
    //         partial::Action::Infer(&Err(SemanticError::DeclarationNotFound(
    //             (
    //                 ShallowType(Type::Module(vec![(str!("foo"), RefKind::Value, ())])),
    //                 NodeId(1)
    //             ),
    //             str!("bar")
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Type), &NodeId(1), "bar"),
    //         partial::Action::Infer(&Err(SemanticError::DeclarationNotFound(
    //             (
    //                 ShallowType(Type::Module(vec![(str!("foo"), RefKind::Value, ())])),
    //                 NodeId(1)
    //             ),
    //             str!("bar")
    //         )))
    //     );
    // }

    #[test]
    fn enumerated_variant_result() {
        let state = strong_state_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Type, Ok(&ReferenceType(Type::Nil)))),
                (
                    NodeId(1),
                    (
                        RefKind::Mixed,
                        Ok(&ReferenceType(Type::Enumerated(Enumerated::Declaration(
                            vec![(str!("Foo"), vec![&ReferenceType(Type::Nil)])],
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&state, &MockNode(RefKind::Value), &NodeId(1), "Foo"),
            partial::Action::Infer(&Ok(&ReferenceType(Type::Enumerated(Enumerated::Variant(
                vec![&ReferenceType(Type::Nil)],
                &ReferenceType(Type::Enumerated(Enumerated::Declaration(vec![(
                    str!("Foo"),
                    vec![&ReferenceType(Type::Nil)]
                )],)))
            )))))
        );
    }

    // #[test]
    // fn enumerated_variant_not_found() {
    //     let state = strong_result_from(
    //         vec![],
    //         vec![(
    //             NodeId(0),
    //             (
    //                 RefKind::Mixed,
    //                 Ok(ScopedType::Type(Type::Enumerated(Enumerated::Declaration(
    //                     vec![(str!("Foo"), vec![])],
    //                 )))),
    //             ),
    //         )],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Value), &NodeId(0), "Bar"),
    //         partial::Action::Infer(&Err(SemanticError::VariantNotFound(
    //             (
    //                 ShallowType(Type::Enumerated(Enumerated::Declaration(vec![(
    //                     str!("Foo"),
    //                     vec![]
    //                 )]))),
    //                 NodeId(0)
    //             ),
    //             str!("Bar")
    //         )))
    //     );
    // }

    // #[test]
    // fn not_indexable() {
    //     let state = strong_result_from(
    //         vec![],
    //         vec![(NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil))))],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&state, &MockNode(RefKind::Value), &NodeId(0), "foo"),
    //         partial::Action::Infer(&Err(SemanticError::NotIndexable(
    //             (ShallowType(Type::Nil), NodeId(0)),
    //             str!("foo")
    //         )))
    //     );
    // }

    #[test]
    fn not_inferrable() {
        let state = strong_state_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Err(ResolveError::NotInferrable(vec![]))),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&state, &MockNode(RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
