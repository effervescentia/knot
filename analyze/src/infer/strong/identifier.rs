use crate::{
    context::StrongContext,
    infer::strong::{SemanticError, Strong},
};
use lang::{
    ast::{explode, walk},
    types,
};

pub fn infer(
    scope: &explode::ScopeId,
    id: &walk::NodeId,
    name: &str,
    kind: &types::RefKind,
    ctx: &StrongContext,
) -> Option<Strong> {
    match ctx.program.bindings.resolve(scope, name, *id) {
        Some(inherit_id) => ctx.as_strong(&inherit_id, kind).cloned(),

        None => Some(Err(SemanticError::NotFound(name.to_owned()))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::StrongContext,
        infer::strong::{SemanticError, Strong},
        test::fixture::strong_ctx_from,
    };
    use kore::str;
    use lang::{
        ast::{explode::ScopeId, walk::NodeId},
        types,
    };
    use std::collections::BTreeSet;

    fn infer(
        scope: &[usize],
        id: usize,
        name: &str,
        kind: types::RefKind,
        ctx: &StrongContext,
    ) -> Option<Strong> {
        super::infer(scope, &id, name, &kind, ctx)
    }

    #[test]
    fn not_found() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(
            infer(&[0], 0, "foo", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::NotFound(str!("foo"))))
        );
    }

    #[test]
    fn found_in_scope() {
        let ctx = strong_ctx_from(
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
            infer(&[0], 2, "foo", types::RefKind::Value, &ctx),
            Some(Ok(types::Type::Boolean))
        );
        assert_eq!(
            infer(&[0], 2, "bar", types::RefKind::Type, &ctx),
            Some(Ok(types::Type::Integer))
        );
    }
}
