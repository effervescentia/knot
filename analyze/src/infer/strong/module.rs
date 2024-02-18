use crate::{
    ast,
    data::ScopedType,
    strong::{Strong, StrongResult},
};
use lang::{types, Fragment, NodeId};

pub fn infer<'a>(declarations: &[NodeId], result: &StrongResult) -> Option<Strong<'a>> {
    let typed_declarations = declarations
        .iter()
        .map(|x| match result.module.fragments.0.get(x)? {
            (
                _,
                Fragment::Declaration(
                    ast::Declaration::TypeAlias { storage, .. }
                    | ast::Declaration::Enumerated { storage, .. }
                    | ast::Declaration::Constant { storage, .. }
                    | ast::Declaration::Function { storage, .. }
                    | ast::Declaration::View { storage, .. }
                    | ast::Declaration::Module { storage, .. },
                ),
            ) => {
                let (kind, _) = result.refs.get(x)?;

                Some((storage.binding, *kind, *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>()?;

    Some(Ok(ScopedType::Type(types::Type::Module(
        typed_declarations,
    ))))
}

#[cfg(test)]
mod tests {
    use crate::{ast, data::ScopedType, test::fixture::strong_result_from};
    use kore::str;
    use lang::{types, Fragment, NodeId, ScopeId};

    #[test]
    fn none_result() {
        let result = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(super::infer(&[NodeId(0)], &result), None);
    }

    #[test]
    fn declarations() {
        let result = strong_result_from(
            vec![
                (
                    NodeId(0),
                    (
                        ScopeId(vec![0]),
                        Fragment::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::public(str!("foo")),
                            value_type: None,
                            value: NodeId(0),
                        }),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        ScopeId(vec![0]),
                        Fragment::Declaration(ast::Declaration::TypeAlias {
                            storage: ast::Storage::public(str!("bar")),
                            value: NodeId(2),
                        }),
                    ),
                ),
            ],
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
            vec![],
        );

        assert_eq!(
            super::infer(&[NodeId(0), NodeId(1)], &result),
            Some(Ok(ScopedType::Type(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, NodeId(0)),
                (str!("bar"), types::RefKind::Type, NodeId(1))
            ]))))
        );
    }

    #[test]
    #[ignore = "not finished"]
    fn imports() {
        let result = strong_result_from(
            vec![
                (
                    NodeId(0),
                    (
                        ScopeId(vec![0]),
                        Fragment::Import(ast::Import {
                            source: ast::ImportSource::Local,
                            path: vec![],
                            alias: None,
                        }),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        ScopeId(vec![0]),
                        Fragment::Declaration(ast::Declaration::Constant {
                            storage: ast::Storage::public(str!("foo")),
                            value_type: None,
                            value: NodeId(2),
                        }),
                    ),
                ),
            ],
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
            vec![],
        );

        assert_eq!(
            super::infer(&[NodeId(0), NodeId(1)], &result),
            Some(Ok(ScopedType::Type(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, NodeId(0)),
                (str!("bar"), types::RefKind::Type, NodeId(1))
            ]))))
        );
    }
}
