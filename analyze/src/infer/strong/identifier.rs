use super::{inherit, partial};
use crate::{
    data::{NodeKind, ResolveTarget},
    error::ResolveError,
    strong,
};

pub fn infer<'a, Node>(strong: &strong::Result, node: &Node, name: &str) -> partial::Action<'a>
where
    Node: ResolveTarget + NodeKind,
{
    match strong.module.bindings.resolve(node, name) {
        Some(from_id) => inherit::inherit(strong, &from_id, node.kind()),

        None => partial::Action::Infer(&Err(ResolveError::NotFound(name.to_string(), *node.id()))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        data::{NodeKind, ResolveTarget, ScopedType},
        error::ResolveError,
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use kore::str;
    use lang::{types, NodeId, ScopeId};
    use std::collections::BTreeSet;

    #[derive(Debug, PartialEq)]
    struct MockNode(types::RefKind, NodeId);

    impl ResolveTarget for MockNode {
        fn id(&self) -> &NodeId {
            &self.1
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

        assert_eq!(
            super::infer(&strong, &MockNode(types::RefKind::Value, NodeId(0)), "foo"),
            partial::Action::Infer(&Err(ResolveError::NotFound(str!("foo"), NodeId(0))))
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
            super::infer(&strong, &MockNode(types::RefKind::Value, NodeId(0)), "foo"),
            partial::Action::Infer(&Ok(ScopedType::Type(types::Type::Boolean)))
        );
        assert_eq!(
            super::infer(&strong, &MockNode(types::RefKind::Type, NodeId(0)), "bar"),
            partial::Action::Infer(&Ok(ScopedType::Type(types::Type::Integer)))
        );
    }
}
