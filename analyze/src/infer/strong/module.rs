use super::{
    data::{Action, Data},
    state::State,
};
use kore::invariant;
use lang::{types::Type, Fragment, NodeId};

pub fn infer(state: &State, declarations: &[NodeId]) -> Action {
    let typed_declarations = declarations
        .iter()
        .map(|x| match state.fragments.get(x)? {
            (_, Fragment::Declaration(declaration)) => {
                let (kind, _) = state.types.get(x)?;

                Some((declaration.binding().clone(), *kind, *x))
            }

            _ => invariant!("fragment should not appear as a child of module"),
        })
        .collect::<Option<Vec<_>>>();

    typed_declarations
        .map(|xs| Action::Infer(Data::Local(Type::Module(xs))))
        .unwrap_or(Action::Skip)
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::{
            strong::{
                data::{Action, Data},
                state::State,
            },
            BindingMap,
        },
    };
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{Kind, Type},
        Fragment, NodeId, ScopeId,
    };
    use std::collections::BTreeMap;

    #[allow(clippy::type_complexity)]
    fn mock_state(
        fragments: &BTreeMap<NodeId, (ScopeId, Fragment)>,
        types: Vec<(NodeId, (Kind, Result<Data, ResolveError>))>,
    ) -> State {
        State {
            fragments,
            bindings: BindingMap::default(),
            nodes: vec![],
            types: BTreeMap::from_iter(types),
            warnings: vec![],
        }
    }

    #[test]
    fn infer_module() {
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
            &fragments,
            vec![
                (NodeId(1), (Kind::Type, Ok(Data::Local(Type::Boolean)))),
                (NodeId(3), (Kind::Value, Ok(Data::Local(Type::Integer)))),
            ],
        );

        assert_eq!(
            super::infer(&state, &[NodeId(1), NodeId(3)]),
            Action::Infer(Data::Local(Type::Module(vec![
                (str!("Foo"), Kind::Type, NodeId(1)),
                (str!("BAR"), Kind::Value, NodeId(3))
            ])))
        );
    }

    #[test]
    fn skip() {
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
            &fragments,
            vec![(NodeId(3), (Kind::Type, Ok(Data::Local(Type::Integer))))],
        );

        assert_eq!(super::infer(&state, &[NodeId(1)]), Action::Skip);
        assert_eq!(super::infer(&state, &[NodeId(1), NodeId(3)]), Action::Skip);
    }
}
