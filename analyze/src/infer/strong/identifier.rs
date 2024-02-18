use crate::{
    data::{NodeKind, ResolveTarget},
    error::SemanticError,
    strong,
};

use super::{inherit, partial};

pub fn infer<'a, Node>(strong: &strong::Result, node: Node, name: &str) -> partial::Action<'a, Node>
where
    Node: ResolveTarget + NodeKind,
{
    match strong.module.bindings.resolve(&node, name) {
        Some(from_id) => inherit::inherit(strong, node, &from_id, node.kind()),

        None => partial::Action::Infer(node, &Err(SemanticError::NotFound(name.to_owned()))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::{NodeKind, ResolveTarget, ScopedType},
        error::SemanticError,
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use kore::str;
    use lang::{types, NodeId, ScopeId};
    use std::collections::BTreeSet;

    #[derive(Debug, PartialEq)]
    struct MockNode(types::RefKind);

    impl ResolveTarget for MockNode {
        fn id(&self) -> &NodeId {
            &NodeId(0)
        }

        fn scope(&self) -> &ScopeId {
            &ScopeId(vec![])
        }
    }

    impl NodeKind for MockNode {
        fn kind(&self) -> &types::RefKind {
            &self.0
        }
    }

    #[test]
    fn not_found() {
        let strong = strong_result_from(vec![], vec![], vec![]);
        let node = || MockNode(types::RefKind::Value);

        assert_eq!(
            super::infer(&strong, node(), "foo"),
            partial::Action::Infer(node(), &Err(SemanticError::NotFound(str!("foo"))))
        );
    }

    #[test]
    fn found_in_scope() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Boolean)),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Type,
                        Ok(ScopedType::Type(types::Type::Integer)),
                    ),
                ),
            ],
            vec![
                (
                    (ScopeId(vec![0]), str!("foo")),
                    BTreeSet::from_iter(vec![NodeId(0)]),
                ),
                (
                    (ScopeId(vec![0]), str!("bar")),
                    BTreeSet::from_iter(vec![NodeId(1)]),
                ),
            ],
        );

        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Value), "foo"),
            partial::Action::Infer(
                MockNode(types::RefKind::Value),
                &Ok(ScopedType::Type(types::Type::Boolean))
            )
        );
        assert_eq!(
            super::infer(&strong, MockNode(types::RefKind::Type), "bar"),
            partial::Action::Infer(
                MockNode(types::RefKind::Type),
                &Ok(ScopedType::Type(types::Type::Integer))
            )
        );
    }
}
