use super::partial;
use crate::{
    data::{NodeKind, ScopedType},
    error::ResolveError,
    strong,
};
use lang::{
    types::{Enumerated, Type},
    NodeId,
};

pub fn infer<'a, Node>(
    strong: &strong::Result,
    node: &Node,
    lhs: &NodeId,
    property: &str,
) -> partial::Action<'a>
where
    Node: NodeKind,
{
    match strong.resolve_type(lhs, node.kind()) {
        Some(Ok(x @ Type::Module(declarations))) => {
            match declarations.iter().find(|(name, ..)| name == property) {
                Some((_, declaration_kind, declaration_id))
                    if node.kind().can_accept(declaration_kind) =>
                {
                    partial::Action::Inherit(&declaration_id)
                }

                Some(_) | None => partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![]))),
            }
        }

        Some(Ok(x @ Type::Enumerated(Enumerated::Declaration(variants)))) => {
            match variants.iter().find(|(name, _)| name == property) {
                Some((_, parameters)) => partial::Action::Infer(&Ok(ScopedType::Type(
                    Type::Enumerated(Enumerated::Variant(parameters.clone(), *lhs)),
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
        data::{NodeKind, ScopedType},
        error::ResolveError,
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use kore::str;
    use lang::{
        types::{Enumerated, RefKind, Type},
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
        let strong = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(&strong, &MockNode(RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Skip
        );
    }

    #[test]
    fn module_entry_result() {
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil)))),
                (NodeId(1), (RefKind::Type, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(2),
                    (
                        RefKind::Mixed,
                        Ok(ScopedType::Type(Type::Module(vec![
                            (str!("foo"), RefKind::Value, NodeId(0)),
                            (str!("bar"), RefKind::Type, NodeId(1)),
                        ]))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &MockNode(RefKind::Value), &NodeId(2), "foo"),
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Nil)))
        );
        assert_eq!(
            super::infer(&strong, &MockNode(RefKind::Type), &NodeId(2), "bar"),
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Nil)))
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
    //     let strong = strong_result_from(
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
    //         super::infer(&strong, &MockNode(RefKind::Type), &NodeId(2), "foo"),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedKind(
    //             (RefKind::Value, NodeId(2)),
    //             RefKind::Type
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&strong, &MockNode(RefKind::Value), &NodeId(2), "bar"),
    //         partial::Action::Infer(&Err(SemanticError::UnexpectedKind(
    //             (RefKind::Type, NodeId(2)),
    //             RefKind::Value
    //         )))
    //     );
    // }

    // #[test]
    // fn module_declaration_not_found() {
    //     let strong = strong_result_from(
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
    //         super::infer(&strong, &MockNode(RefKind::Value), &NodeId(1), "bar"),
    //         partial::Action::Infer(&Err(SemanticError::DeclarationNotFound(
    //             (
    //                 ShallowType(Type::Module(vec![(str!("foo"), RefKind::Value, ())])),
    //                 NodeId(1)
    //             ),
    //             str!("bar")
    //         )))
    //     );
    //     assert_eq!(
    //         super::infer(&strong, &MockNode(RefKind::Type), &NodeId(1), "bar"),
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
        let strong = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Type, Ok(ScopedType::Type(Type::Nil)))),
                (
                    NodeId(1),
                    (
                        RefKind::Mixed,
                        Ok(ScopedType::Type(Type::Enumerated(Enumerated::Declaration(
                            vec![(str!("Foo"), vec![NodeId(0)])],
                        )))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, &MockNode(RefKind::Value), &NodeId(1), "Foo"),
            partial::Action::Infer(&Ok(ScopedType::Type(Type::Enumerated(
                Enumerated::Variant(vec![NodeId(0)], NodeId(1))
            ))))
        );
    }

    // #[test]
    // fn enumerated_variant_not_found() {
    //     let strong = strong_result_from(
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
    //         super::infer(&strong, &MockNode(RefKind::Value), &NodeId(0), "Bar"),
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
    //     let strong = strong_result_from(
    //         vec![],
    //         vec![(NodeId(0), (RefKind::Value, Ok(ScopedType::Type(Type::Nil))))],
    //         vec![],
    //     );

    //     assert_eq!(
    //         super::infer(&strong, &MockNode(RefKind::Value), &NodeId(0), "foo"),
    //         partial::Action::Infer(&Err(SemanticError::NotIndexable(
    //             (ShallowType(Type::Nil), NodeId(0)),
    //             str!("foo")
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
            super::infer(&strong, &MockNode(RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
