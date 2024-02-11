use crate::{
    context::StrongContext,
    infer::strong::{SemanticError, Strong},
};
use lang::{ast::walk, types};

pub fn infer(
    lhs: walk::NodeId,
    rhs: String,
    kind: &types::RefKind,
    ctx: &StrongContext,
) -> Option<Strong> {
    match ctx.as_strong(&lhs, kind)? {
        Ok(x @ types::Type::Module(declarations)) => {
            match declarations.iter().find(|(name, ..)| name == &rhs) {
                Some((_, declaration_kind, declaration_id))
                    if declaration_kind == kind || declaration_kind == &types::RefKind::Mixed =>
                {
                    ctx.as_strong(declaration_id, kind).cloned()
                }

                Some(_) => Some(Err(SemanticError::IllegalTypeAccess(
                    (x.clone(), lhs),
                    rhs.clone(),
                ))),

                None => Some(Err(SemanticError::DeclarationNotFound(
                    (x.clone(), lhs),
                    rhs.clone(),
                ))),
            }
        }

        Ok(x @ types::Type::Enumerated(types::Enumerated::Declaration(variants))) => {
            match variants.iter().find(|(name, _)| name == &rhs) {
                Some((_, parameters)) => Some(Ok(types::Type::Enumerated(
                    types::Enumerated::Variant(parameters.clone(), lhs),
                ))),

                None => Some(Err(SemanticError::VariantNotFound((x.clone(), lhs), rhs))),
            }
        }

        Ok(x) => Some(Err(SemanticError::NotIndexable(
            (x.clone(), lhs),
            rhs.clone(),
        ))),

        Err(_) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),
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
    use lang::{ast::walk::NodeId, types};

    fn infer(lhs: usize, rhs: &str, kind: types::RefKind, ctx: &StrongContext) -> Option<Strong> {
        super::infer(NodeId(lhs), rhs.to_owned(), &kind, ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(infer(0, "foo", types::RefKind::Value, &ctx), None);
    }

    #[test]
    fn module_entry_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Nil))),
                (
                    NodeId(2),
                    (
                        types::RefKind::Mixed,
                        Ok(types::Type::Module(vec![
                            (str!("foo"), types::RefKind::Value, NodeId(0)),
                            (str!("bar"), types::RefKind::Type, NodeId(1)),
                        ])),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            infer(2, "foo", types::RefKind::Value, &ctx),
            Some(Ok(types::Type::Nil))
        );
        assert_eq!(
            infer(2, "bar", types::RefKind::Type, &ctx),
            Some(Ok(types::Type::Nil))
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
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Nil))),
                (NodeId(2), (types::RefKind::Mixed, Ok(module_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(2, "foo", types::RefKind::Type, &ctx),
            Some(Err(SemanticError::IllegalTypeAccess(
                (module_type(), NodeId(2)),
                str!("foo")
            )))
        );
        assert_eq!(
            infer(2, "bar", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::IllegalTypeAccess(
                (module_type(), NodeId(2)),
                str!("bar")
            )))
        );
    }

    #[test]
    fn module_declaration_not_found() {
        let module_type =
            || types::Type::Module(vec![(str!("foo"), types::RefKind::Value, NodeId(0))]);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Mixed, Ok(module_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(1, "bar", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), NodeId(1)),
                str!("bar")
            )))
        );
        assert_eq!(
            infer(1, "bar", types::RefKind::Type, &ctx),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), NodeId(1)),
                str!("bar")
            )))
        );
    }

    #[test]
    fn enumerated_variant_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Type, Ok(types::Type::Nil))),
                (
                    NodeId(1),
                    (
                        types::RefKind::Mixed,
                        Ok(types::Type::Enumerated(types::Enumerated::Declaration(
                            vec![(str!("Foo"), vec![NodeId(0)])],
                        ))),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            infer(1, "Foo", types::RefKind::Value, &ctx),
            Some(Ok(types::Type::Enumerated(types::Enumerated::Variant(
                vec![NodeId(0)],
                NodeId(1)
            ))))
        );
    }

    #[test]
    fn enumerated_variant_not_found() {
        let enum_type =
            || types::Type::Enumerated(types::Enumerated::Declaration(vec![(str!("Foo"), vec![])]));
        let ctx = strong_ctx_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Mixed, Ok(enum_type())))],
            vec![],
        );

        assert_eq!(
            infer(0, "Bar", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::VariantNotFound(
                (enum_type(), NodeId(0)),
                str!("Bar")
            )))
        );
    }

    #[test]
    fn not_indexable() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil)))],
            vec![],
        );

        assert_eq!(
            infer(0, "foo", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::NotIndexable(
                (types::Type::Nil, NodeId(0)),
                str!("foo")
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let ctx = strong_ctx_from(
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
            infer(0, "foo", types::RefKind::Value, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
