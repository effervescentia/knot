use crate::{
    context::StrongContext,
    infer::strong::{SemanticError, Strong},
    RefKind,
};

pub fn infer(
    scope: &[usize],
    id: &usize,
    name: &str,
    kind: &RefKind,
    ctx: &StrongContext,
) -> Option<Strong> {
    match ctx.bindings.resolve(scope, name, *id) {
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
        types::Type,
        RefKind,
    };
    use kore::str;
    use std::collections::BTreeSet;

    fn infer(
        scope: &[usize],
        id: usize,
        name: &str,
        kind: RefKind,
        ctx: &StrongContext,
    ) -> Option<Strong> {
        super::infer(scope, &id, name, &kind, ctx)
    }

    #[test]
    fn not_found() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(
            infer(&[0], 0, "foo", RefKind::Value, &ctx),
            Some(Err(SemanticError::NotFound(str!("foo"))))
        );
    }

    #[test]
    fn found_in_scope() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Boolean))),
                (1, (RefKind::Type, Ok(Type::Integer))),
            ],
            vec![
                ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
                ((vec![0], str!("bar")), BTreeSet::from_iter(vec![1])),
            ],
        );

        assert_eq!(
            infer(&[0], 2, "foo", RefKind::Value, &ctx),
            Some(Ok(Type::Boolean))
        );
        assert_eq!(
            infer(&[0], 2, "bar", RefKind::Type, &ctx),
            Some(Ok(Type::Integer))
        );
    }
}
