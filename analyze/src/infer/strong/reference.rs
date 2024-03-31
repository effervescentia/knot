use super::{data::Action, inherit, state::State};
use crate::{error::ResolveError, infer::NodeDescriptor};

pub fn infer(state: &State, name: &str, node: &NodeDescriptor) -> Action {
    println!(
        "RESOLVING INFERENCE {name} {node:?} {:?}",
        state.bindings.resolve(node, name)
    );
    match state.bindings.resolve(node, name) {
        Some(from_id) => inherit::inherit(state, from_id, &node.kind),

        None => Action::Raise(ResolveError::NotFound(name.to_owned(), node.id)),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::{
            strong::{
                data::{Action, Type},
                mock::FRAGMENTS,
                state::State,
            },
            weak, BindingMap, NodeDescriptor,
        },
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Kind},
        NodeId, ScopeId,
    };
    use std::collections::{BTreeMap, BTreeSet, HashMap};

    #[allow(clippy::type_complexity)]
    fn mock_state<'a>(
        bindings: Vec<((ScopeId, String), BTreeSet<NodeId>)>,
        types: Vec<(NodeId, (Kind, Result<Type, ResolveError>))>,
    ) -> State<'a> {
        State {
            fragments: FRAGMENTS,
            bindings: BindingMap(HashMap::from_iter(bindings)),
            nodes: vec![],
            types: BTreeMap::from_iter(types),
            warnings: vec![],
        }
    }

    #[test]
    fn inherit() {
        let node = NodeDescriptor {
            id: NodeId(2),
            scope: ScopeId(vec![0]),
            kind: Kind::Value,
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"))),
        };
        let state = mock_state(
            vec![(
                (ScopeId(vec![0]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(1)]),
            )],
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Local(types::Type::Integer))),
            )],
        );

        assert_eq!(
            super::infer(&state, "foo", &node),
            Action::Infer(Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn not_found() {
        let node = NodeDescriptor {
            id: NodeId(1),
            scope: ScopeId(vec![0]),
            kind: Kind::Value,
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"))),
        };
        let state = mock_state(vec![], vec![]);

        assert_eq!(
            super::infer(&state, "foo", &node),
            Action::Raise(ResolveError::NotFound(str!("foo"), NodeId(1)))
        );
    }
}
