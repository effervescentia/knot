use super::{
    data::{Action, Type},
    state::State,
};
use kore::invariant;
use lang::{types, Canonicalize, Fragment, NodeId};

pub fn infer(state: &State, declarations: &[NodeId]) -> Action {
    let typed_declarations = declarations
        .iter()
        .map(|x| match state.fragments.get(x)? {
            (_, Fragment::Declaration(declaration)) => {
                let (kind, _) = state.types.get(x)?;

                Some((declaration.binding().clone(), *kind, state.canonicalize(*x)))
            }

            (_, Fragment::TypeDeclaration(declaration)) => {
                let (kind, _) = state.types.get(x)?;

                Some((declaration.binding().clone(), *kind, state.canonicalize(*x)))
            }

            _ => invariant!("fragment should not appear as a child of module"),
        })
        .collect::<Option<Vec<_>>>();

    typed_declarations
        .map(|xs| Action::Infer(Type::Value(types::Type::Module(xs))))
        .unwrap_or(Action::Skip)
}

#[cfg(test)]
mod tests {
    use crate::{
        analyze_mock,
        error::Error,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
        Context,
    };
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{self, Kind},
        CanonicalId, Fragment, NodeId, ScopeId,
    };
    use std::collections::BTreeMap;

    #[allow(clippy::type_complexity)]
    fn mock_state<'a>(
        ctx: &'a Context,
        fragments: &'a BTreeMap<NodeId, (ScopeId, Fragment)>,
        types: Vec<(NodeId, (Kind, Result<Type, Error>))>,
    ) -> State<'a> {
        State {
            fragments,
            types: BTreeMap::from_iter(types),
            ..State::mock(ctx)
        }
    }

    #[test]
    fn infer_module() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let fragments = BTreeMap::from_iter(vec![
            (
                NodeId(1),
                (
                    ScopeId(vec![]),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("Foo")),
                        NodeId(2),
                    )),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![]),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("BAR")),
                        None,
                        NodeId(4),
                    )),
                ),
            ),
        ]);
        let state = mock_state(
            &ctx,
            &fragments,
            vec![
                (
                    NodeId(1),
                    (Kind::Type, Ok(Type::Value(types::Type::Boolean))),
                ),
                (
                    NodeId(3),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, &[NodeId(1), NodeId(3)]),
            Action::Infer(Type::Value(types::Type::Module(vec![
                (str!("Foo"), Kind::Type, CanonicalId::mock(1)),
                (str!("BAR"), Kind::Value, CanonicalId::mock(3))
            ])))
        );
    }

    #[test]
    fn skip() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let fragments = BTreeMap::from_iter(vec![
            (
                NodeId(1),
                (
                    ScopeId(vec![]),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("Foo")),
                        NodeId(2),
                    )),
                ),
            ),
            (
                NodeId(3),
                (
                    ScopeId(vec![]),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("Bar")),
                        NodeId(4),
                    )),
                ),
            ),
        ]);
        let state = mock_state(
            &ctx,
            &fragments,
            vec![(
                NodeId(3),
                (Kind::Type, Ok(Type::Value(types::Type::Integer))),
            )],
        );

        assert_eq!(super::infer(&state, &[NodeId(1)]), Action::Skip);
        assert_eq!(super::infer(&state, &[NodeId(1), NodeId(3)]), Action::Skip);
    }
}
