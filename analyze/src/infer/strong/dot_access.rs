use crate::{
    context::StrongContext,
    infer::strong::{SemanticError, Strong},
    RefKind, Type,
};

pub fn infer(lhs: usize, rhs: String, kind: &RefKind, ctx: &StrongContext) -> Option<Strong> {
    match ctx.as_strong(&lhs, kind)? {
        Ok(x @ Type::Module(declarations)) => {
            match declarations.iter().find(|(name, ..)| name == &rhs) {
                Some((_, declaration_kind, declaration_id))
                    if declaration_kind == kind || declaration_kind == &RefKind::Mixed =>
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

        Ok(x @ Type::Enumerated(variants)) => {
            match variants.iter().find(|(name, _)| name == &rhs) {
                Some((_, parameters)) => Some(Ok(Type::EnumeratedVariant(parameters.clone(), lhs))),

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
        types::Type,
        RefKind,
    };
    use kore::str;

    fn infer(lhs: usize, rhs: &str, kind: RefKind, ctx: &StrongContext) -> Option<Strong> {
        super::infer(lhs, rhs.to_owned(), &kind, ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(infer(0, "foo", RefKind::Value, &ctx), None);
    }

    #[test]
    fn module_entry_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Type, Ok(Type::Nil))),
                (
                    2,
                    (
                        RefKind::Mixed,
                        Ok(Type::Module(vec![
                            (str!("foo"), RefKind::Value, 0),
                            (str!("bar"), RefKind::Type, 1),
                        ])),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(infer(2, "foo", RefKind::Value, &ctx), Some(Ok(Type::Nil)));
        assert_eq!(infer(2, "bar", RefKind::Type, &ctx), Some(Ok(Type::Nil)));
    }

    #[test]
    fn module_illegal_type_access() {
        let module_type = || {
            Type::Module(vec![
                (str!("foo"), RefKind::Value, 0),
                (str!("bar"), RefKind::Type, 1),
            ])
        };
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Type, Ok(Type::Nil))),
                (2, (RefKind::Mixed, Ok(module_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(2, "foo", RefKind::Type, &ctx),
            Some(Err(SemanticError::IllegalTypeAccess(
                (module_type(), 2),
                str!("foo")
            )))
        );
        assert_eq!(
            infer(2, "bar", RefKind::Value, &ctx),
            Some(Err(SemanticError::IllegalTypeAccess(
                (module_type(), 2),
                str!("bar")
            )))
        );
    }

    #[test]
    fn module_declaration_not_found() {
        let module_type = || Type::Module(vec![(str!("foo"), RefKind::Value, 0)]);
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Nil))),
                (1, (RefKind::Mixed, Ok(module_type()))),
            ],
            vec![],
        );

        assert_eq!(
            infer(1, "bar", RefKind::Value, &ctx),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), 1),
                str!("bar")
            )))
        );
        assert_eq!(
            infer(1, "bar", RefKind::Type, &ctx),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), 1),
                str!("bar")
            )))
        );
    }

    #[test]
    fn enumerated_variant_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Type, Ok(Type::Nil))),
                (
                    1,
                    (
                        RefKind::Mixed,
                        Ok(Type::Enumerated(vec![(str!("Foo"), vec![0])])),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            infer(1, "Foo", RefKind::Value, &ctx),
            Some(Ok(Type::EnumeratedVariant(vec![0], 1)))
        );
    }

    #[test]
    fn enumerated_variant_not_found() {
        let enum_type = || Type::Enumerated(vec![(str!("Foo"), vec![])]);
        let ctx = strong_ctx_from(vec![], vec![(0, (RefKind::Mixed, Ok(enum_type())))], vec![]);

        assert_eq!(
            infer(0, "Bar", RefKind::Value, &ctx),
            Some(Err(SemanticError::VariantNotFound(
                (enum_type(), 0),
                str!("Bar")
            )))
        );
    }

    #[test]
    fn not_indexable() {
        let ctx = strong_ctx_from(vec![], vec![(0, (RefKind::Value, Ok(Type::Nil)))], vec![]);

        assert_eq!(
            infer(0, "foo", RefKind::Value, &ctx),
            Some(Err(SemanticError::NotIndexable(
                (Type::Nil, 0),
                str!("foo")
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(
                0,
                (RefKind::Value, Err(SemanticError::NotInferrable(vec![]))),
            )],
            vec![],
        );

        assert_eq!(
            infer(0, "foo", RefKind::Value, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![0])))
        );
    }
}
