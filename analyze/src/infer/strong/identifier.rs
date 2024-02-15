use crate::{
    infer::strong::SemanticError,
    strong::{Strong, StrongResult},
};
use lang::{types, NodeId, ScopeId};

pub fn infer(
    scope: &ScopeId,
    id: &NodeId,
    name: &str,
    kind: &types::RefKind,
    result: &StrongResult,
) -> Option<Strong> {
    match result.module.bindings.resolve(scope, name, *id) {
        Some(inherit_id) => result.as_strong(&inherit_id, kind).cloned(),

        None => Some(Err(SemanticError::NotFound(name.to_owned()))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{infer::strong::SemanticError, test::fixture::strong_result_from};
    use kore::str;
    use lang::{types, NodeId, ScopeId};
    use std::collections::BTreeSet;

    #[test]
    fn not_found() {
        let result = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(
                &ScopeId(vec![0]),
                &NodeId(0),
                "foo",
                &types::RefKind::Value,
                &result
            ),
            Some(Err(SemanticError::NotFound(str!("foo"))))
        );
    }

    #[test]
    fn found_in_scope() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Boolean))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Integer))),
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
            super::infer(
                &ScopeId(vec![0]),
                &NodeId(2),
                "foo",
                &types::RefKind::Value,
                &result
            ),
            Some(Ok(types::Type::Boolean))
        );
        assert_eq!(
            super::infer(
                &ScopeId(vec![0]),
                &NodeId(2),
                "bar",
                &types::RefKind::Type,
                &result
            ),
            Some(Ok(types::Type::Integer))
        );
    }
}
