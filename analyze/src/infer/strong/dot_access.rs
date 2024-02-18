use crate::{
    data::ScopedType,
    error::SemanticError,
    strong::{Strong, StrongResult},
};
use lang::{types, NodeId};

pub fn infer<'a>(
    lhs: &NodeId,
    rhs: &str,
    kind: &types::RefKind,
    result: &StrongResult,
) -> Option<Strong<'a>> {
    match result.as_strong(lhs, kind)? {
        Ok(ScopedType::Type(x @ types::Type::Module(declarations))) => {
            match declarations.iter().find(|(name, ..)| name == rhs) {
                Some((_, declaration_kind, declaration_id))
                    if declaration_kind == kind || declaration_kind == &types::RefKind::Mixed =>
                {
                    result.as_strong(declaration_id, kind).cloned()
                }

                Some(_) => Some(Err(SemanticError::IllegalTypeAccess(
                    (x.clone(), *lhs),
                    rhs.to_string(),
                ))),

                None => Some(Err(SemanticError::DeclarationNotFound(
                    (x.clone(), *lhs),
                    rhs.to_string(),
                ))),
            }
        }

        Ok(ScopedType::Type(
            x @ types::Type::Enumerated(types::Enumerated::Declaration(variants)),
        )) => match variants.iter().find(|(name, _)| name == rhs) {
            Some((_, parameters)) => Some(Ok(ScopedType::Type(types::Type::Enumerated(
                types::Enumerated::Variant(parameters.clone(), *lhs),
            )))),

            None => Some(Err(SemanticError::VariantNotFound(
                (x.clone(), *lhs),
                rhs.to_string(),
            ))),
        },

        Ok(ScopedType::Type(x)) => Some(Err(SemanticError::NotIndexable(
            (x.clone(), *lhs),
            rhs.to_string(),
        ))),

        Err(_) => Some(Err(SemanticError::NotInferrable(vec![*lhs]))),
    }
}

#[cfg(test)]
mod tests {
    use crate::{data::ScopedType, error::SemanticError, test::fixture::strong_result_from};
    use kore::str;
    use lang::{types, NodeId};

    #[test]
    fn none_result() {
        let result = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(
            super::infer(&NodeId(0), "foo", &types::RefKind::Value, &result),
            None
        );
    }

    #[test]
    fn module_entry_result() {
        let result = strong_result_from(
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
            super::infer(&NodeId(2), "foo", &types::RefKind::Value, &result),
            Some(Ok(ScopedType::Type(types::Type::Nil)))
        );
        assert_eq!(
            super::infer(&NodeId(2), "bar", &types::RefKind::Type, &result),
            Some(Ok(ScopedType::Type(types::Type::Nil)))
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
        let result = strong_result_from(
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
                    (types::RefKind::Mixed, Ok(ScopedType::Type(module_type()))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(2), "foo", &types::RefKind::Type, &result),
            Some(Err(SemanticError::IllegalTypeAccess(
                (module_type(), NodeId(2)),
                str!("foo")
            )))
        );
        assert_eq!(
            super::infer(&NodeId(2), "bar", &types::RefKind::Value, &result),
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
        let result = strong_result_from(
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
                    (types::RefKind::Mixed, Ok(ScopedType::Type(module_type()))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(1), "bar", &types::RefKind::Value, &result),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), NodeId(1)),
                str!("bar")
            )))
        );
        assert_eq!(
            super::infer(&NodeId(1), "bar", &types::RefKind::Type, &result),
            Some(Err(SemanticError::DeclarationNotFound(
                (module_type(), NodeId(1)),
                str!("bar")
            )))
        );
    }

    #[test]
    fn enumerated_variant_result() {
        let result = strong_result_from(
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
            super::infer(&NodeId(1), "Foo", &types::RefKind::Value, &result),
            Some(Ok(ScopedType::Type(types::Type::Enumerated(
                types::Enumerated::Variant(vec![NodeId(0)], NodeId(1))
            ))))
        );
    }

    #[test]
    fn enumerated_variant_not_found() {
        let enum_type =
            || types::Type::Enumerated(types::Enumerated::Declaration(vec![(str!("Foo"), vec![])]));
        let result = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (types::RefKind::Mixed, Ok(ScopedType::Type(enum_type()))),
            )],
            vec![],
        );

        assert_eq!(
            super::infer(&NodeId(0), "Bar", &types::RefKind::Value, &result),
            Some(Err(SemanticError::VariantNotFound(
                (enum_type(), NodeId(0)),
                str!("Bar")
            )))
        );
    }

    #[test]
    fn not_indexable() {
        let result = strong_result_from(
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
            super::infer(&NodeId(0), "foo", &types::RefKind::Value, &result),
            Some(Err(SemanticError::NotIndexable(
                (types::Type::Nil, NodeId(0)),
                str!("foo")
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let result = strong_result_from(
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
            super::infer(&NodeId(0), "foo", &types::RefKind::Value, &result),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(0)])))
        );
    }
}
