use super::partial;
use crate::{ast, data::ScopedType, strong};
use lang::{types::Type, Fragment, NodeId};

pub fn infer<'a>(strong: &strong::Result, declarations: &[NodeId]) -> partial::Action<'a> {
    let typed_declarations = declarations
        .iter()
        .map(|x| match strong.module.fragments.0.get(x)? {
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
                let (kind, _) = strong.refs.get(x)?;

                Some((storage.binding, *kind, *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>();

    typed_declarations
        .map(|xs| partial::Action::Infer(&Ok(ScopedType::Type(Type::Module(xs)))))
        .unwrap_or(partial::Action::Skip)
}

#[cfg(test)]
mod tests {
    use crate::{ast, data::ScopedType, infer::strong::partial, test::fixture::strong_result_from};
    use kore::str;
    use lang::{types, Fragment, NodeId, ScopeId};

    #[test]
    fn none_result() {
        let strong = strong_result_from(vec![], vec![], vec![]);

        assert_eq!(super::infer(&strong, &[NodeId(0)]), partial::Action::Skip);
    }

    #[test]
    fn declarations() {
        let strong = strong_result_from(
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
            super::infer(&strong, &[NodeId(0), NodeId(1)],),
            partial::Action::Infer(&Ok(ScopedType::Type(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, NodeId(0)),
                (str!("bar"), types::RefKind::Type, NodeId(1))
            ]))))
        );
    }

    #[test]
    #[ignore = "not finished"]
    fn imports() {
        let strong = strong_result_from(
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
            super::infer(&strong, &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Ok(ScopedType::Type(types::Type::Module(vec![
                (str!("foo"), types::RefKind::Value, NodeId(0)),
                (str!("bar"), types::RefKind::Type, NodeId(1))
            ]))))
        );
    }
}
