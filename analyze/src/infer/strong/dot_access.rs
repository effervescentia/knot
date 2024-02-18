use crate::{
    data::{NodeKind, ScopedType},
    error::SemanticError,
    strong,
};
use lang::{types, NodeId};

use super::partial;

pub fn infer<'a, Node>(
    strong: &strong::Result,
    node: Node,
    lhs: &NodeId,
    rhs: &str,
) -> partial::Action<'a, Node>
where
    Node: NodeKind,
{
    match strong.get_deep_type(lhs, node.kind()) {
        Some(Ok(x @ types::Type::Module(declarations))) => {
            match declarations.iter().find(|(name, ..)| name == rhs) {
                Some((_, declaration_kind, declaration_id))
                    if declaration_kind == node.kind()
                        || declaration_kind == &types::RefKind::Mixed =>
                {
                    match strong.get_shallow_type(declaration_id, node.kind()) {
                        Some(x) => partial::Action::Infer(node, x),

                        None => partial::Action::Skip(node),
                    }
                }

                Some(_) => partial::Action::Infer(
                    node,
                    &Err(SemanticError::IllegalTypeAccess(
                        (x.to_shape(), *lhs),
                        rhs.to_string(),
                    )),
                ),

                None => partial::Action::Infer(
                    node,
                    &Err(SemanticError::DeclarationNotFound(
                        (x.to_shape(), *lhs),
                        rhs.to_string(),
                    )),
                ),
            }
        }

        Some(Ok(x @ types::Type::Enumerated(types::Enumerated::Declaration(variants)))) => {
            match variants.iter().find(|(name, _)| name == rhs) {
                Some((_, parameters)) => partial::Action::Infer(
                    node,
                    &Ok(ScopedType::Type(types::Type::Enumerated(
                        types::Enumerated::Variant(parameters.clone(), *lhs),
                    ))),
                ),

                None => partial::Action::Infer(
                    node,
                    &Err(SemanticError::VariantNotFound(
                        (x.to_shape(), *lhs),
                        rhs.to_string(),
                    )),
                ),
            }
        }

        Some(Ok(x)) => partial::Action::Infer(
            node,
            &Err(SemanticError::NotIndexable(
                (x.to_shape(), *lhs),
                rhs.to_string(),
            )),
        ),

        Some(Err(_)) => {
            partial::Action::Infer(node, &Err(SemanticError::NotInferrable(vec![*lhs])))
        }

        None => partial::Action::Skip(node),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::{NodeKind, ScopedType},
        error::SemanticError,
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use kore::str;
    use lang::{types, NodeId};

    #[derive(Debug, PartialEq)]
    struct MockNode(types::RefKind);

    impl NodeKind for MockNode {
        fn kind(&self) -> &types::RefKind {
            &self.0
        }
    }

    #[test]
    fn skip_infer() {
        let strong = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Skip(MockNode(types::RefKind::Value))
        );
    }

    #[test]
    fn module_entry_result() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil)),
                    ),
                ),
                (
                    NodeId(1),
                    (types::RefKind::Type, Ok(ScopedType::Type(types::Type::Nil))),
                ),
                (
                    NodeId(2),
                    (
                        types::RefKind::Mixed,
                        Ok(ScopedType::Type(types::Type::Module(vec![
                            (str!("foo"), types::RefKind::Value, NodeId(0)),
                            (str!("bar"), types::RefKind::Type, NodeId(1)),
                        ]))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(2), "foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Ok(ScopedType::Type(types::Type::Nil))
            )
        );
        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Type), &NodeId(2), "bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Type),
                &Ok(ScopedType::Type(types::Type::Nil))
            )
        );
    }

    #[test]
    fn module_illegal_type_access() {
        let module_type = || {
            types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, NodeId(0)),
                (str!("bar"), types::RefKind::Type, NodeId(1)),
            ])
        };
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil)),
                    ),
                ),
                (
                    NodeId(1),
                    (types::RefKind::Type, Ok(ScopedType::Type(types::Type::Nil))),
                ),
                (
                    NodeId(2),
                    (
                        types::RefKind::Mixed,
                        Ok(ScopedType::Type(types::Type::Module(vec![
                            (str!("foo"), types::RefKind::Value, NodeId(0)),
                            (str!("bar"), types::RefKind::Type, NodeId(1)),
                        ]))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Type), &NodeId(2), "foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Type),
                &Err(SemanticError::IllegalTypeAccess(
                    (
                        types::TypeShape(types::Type::Module(vec![
                            (str!("foo"), types::RefKind::Value, ()),
                            (str!("bar"), types::RefKind::Type, ()),
                        ])),
                        NodeId(2)
                    ),
                    str!("foo")
                ))
            )
        );
        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(2), "bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Err(SemanticError::IllegalTypeAccess(
                    (
                        types::TypeShape(types::Type::Module(vec![
                            (str!("foo"), types::RefKind::Value, ()),
                            (str!("bar"), types::RefKind::Type, ()),
                        ])),
                        NodeId(2)
                    ),
                    str!("bar")
                ))
            )
        );
    }

    #[test]
    fn module_declaration_not_found() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil)),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Mixed,
                        Ok(ScopedType::Type(types::Type::Module(vec![(
                            str!("foo"),
                            types::RefKind::Value,
                            NodeId(0),
                        )]))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(1), "bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Err(SemanticError::DeclarationNotFound(
                    (
                        types::TypeShape(types::Type::Module(vec![(
                            str!("foo"),
                            types::RefKind::Value,
                            ()
                        )])),
                        NodeId(1)
                    ),
                    str!("bar")
                ))
            )
        );
        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Type), &NodeId(1), "bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Type),
                &Err(SemanticError::DeclarationNotFound(
                    (
                        types::TypeShape(types::Type::Module(vec![(
                            str!("foo"),
                            types::RefKind::Value,
                            ()
                        )])),
                        NodeId(1)
                    ),
                    str!("bar")
                ))
            )
        );
    }

    #[test]
    fn enumerated_variant_result() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (types::RefKind::Type, Ok(ScopedType::Type(types::Type::Nil))),
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Mixed,
                        Ok(ScopedType::Type(types::Type::Enumerated(
                            types::Enumerated::Declaration(vec![(str!("Foo"), vec![NodeId(0)])]),
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(1), "Foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Ok(ScopedType::Type(types::Type::Enumerated(
                    types::Enumerated::Variant(vec![NodeId(0)], NodeId(1))
                )))
            )
        );
    }

    #[test]
    fn enumerated_variant_not_found() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (
                    types::RefKind::Mixed,
                    Ok(ScopedType::Type(types::Type::Enumerated(
                        types::Enumerated::Declaration(vec![(str!("Foo"), vec![])]),
                    ))),
                ),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(0), "Bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Err(SemanticError::VariantNotFound(
                    (
                        types::TypeShape(types::Type::Enumerated(types::Enumerated::Declaration(
                            vec![(str!("Foo"), vec![])]
                        ))),
                        NodeId(0)
                    ),
                    str!("Bar")
                ))
            )
        );
    }

    #[test]
    fn not_indexable() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (
                    types::RefKind::Value,
                    Ok(ScopedType::Type(types::Type::Nil)),
                ),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Err(SemanticError::NotIndexable(
                    (types::TypeShape(types::Type::Nil), NodeId(0)),
                    str!("foo")
                ))
            )
        );
    }

    #[test]
    fn not_inferrable() {
        let strong = strong_result_from(
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
            super::infer(&strong, MockNode(types::RefKind::Value), &NodeId(0), "foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Err(SemanticError::NotInferrable(vec![NodeId(0)]))
            )
        );
    }
}
