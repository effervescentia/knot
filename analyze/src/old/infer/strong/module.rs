use super::partial;
use crate::{ast, strong};
use lang::{
    types::{ReferenceType, Type},
    Fragment, NodeId,
};

pub fn infer<'a>(state: &strong::State, declarations: &[NodeId]) -> partial::Action<'a> {
    let typed_declarations = declarations
        .iter()
        .map(|x| match state.module.fragments.0.get(x)? {
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
                let (kind, _) = state.refs.get(x)?;

                Some((storage.binding, *kind, *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>();

    typed_declarations
        .map(|xs| partial::Action::Infer(&Ok(&ReferenceType(Type::Module(xs)))))
        .unwrap_or(partial::Action::Skip)
}

#[cfg(test)]
mod tests {
    use crate::{ast, infer::strong::partial, test::fixture::strong_state_from};
    use kore::str;
    use lang::{
        types::{RefKind, ReferenceType, Type},
        Fragment, NodeId, ScopeId,
    };

    #[test]
    fn none_result() {
        let state = strong_state_from(vec![], vec![], vec![]);

        assert_eq!(super::infer(&state, &[NodeId(0)]), partial::Action::Skip);
    }

    #[test]
    fn declarations() {
        let state = strong_state_from(
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
                    (RefKind::Value, Ok(&ReferenceType(Type::Boolean))),
                ),
                (
                    NodeId(1),
                    (RefKind::Type, Ok(&ReferenceType(Type::Integer))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&state, &[NodeId(0), NodeId(1)],),
            partial::Action::Infer(&Ok(&ReferenceType(Type::Module(vec![
                (str!("foo"), RefKind::Value, NodeId(0)),
                (str!("bar"), RefKind::Type, NodeId(1))
            ]))))
        );
    }

    #[test]
    #[ignore = "not finished"]
    fn imports() {
        let state = strong_state_from(
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
                    (RefKind::Value, Ok(&ReferenceType(Type::Boolean))),
                ),
                (
                    NodeId(1),
                    (RefKind::Type, Ok(&ReferenceType(Type::Integer))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&state, &[NodeId(0), NodeId(1)]),
            partial::Action::Infer(&Ok(&ReferenceType(Type::Module(vec![
                (str!("foo"), RefKind::Value, NodeId(0)),
                (str!("bar"), RefKind::Type, NodeId(1))
            ]))))
        );
    }
}
