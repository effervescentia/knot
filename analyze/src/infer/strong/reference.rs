use super::{
    data::{Action, Type},
    inherit,
    state::State,
};
use crate::{error::Error, infer::NodeDescriptor, AmbientScope};
use lang::Canonicalize;

pub fn infer(
    state: &State,
    scope: &Option<AmbientScope>,
    name: &str,
    node: &NodeDescriptor,
) -> Action {
    if let Some(from_id) = state.bindings.resolve(node, name) {
        return inherit::inherit(state, state.canonicalize(from_id), &node.kind);
    }

    if let Some(ambient) = scope.and_then(|x| state.resolve_ambient(&x, name)) {
        return Action::Infer(Type::Inherit(ambient.0));
    }

    Action::Raise(Error::NotFound(name.to_owned()))
}

#[cfg(test)]
mod tests {
    use crate::{
        analyze_mock,
        error::Error,
        infer::{
            strong::{
                data::{Action, Type},
                state::State,
            },
            weak, BindingMap, NodeDescriptor,
        },
        Context,
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Kind},
        CanonicalId, NodeId, ScopeId,
    };
    use std::collections::{BTreeMap, BTreeSet, HashMap};

    #[allow(clippy::type_complexity)]
    fn mock_state<'a>(
        ctx: &'a Context,
        bindings: Vec<((ScopeId, String), BTreeSet<NodeId>)>,
        types: Vec<(NodeId, (Kind, Result<Type, Error>))>,
    ) -> State<'a> {
        State {
            bindings: BindingMap(HashMap::from_iter(bindings)),
            types: BTreeMap::from_iter(types),
            ..State::mock(ctx)
        }
    }

    #[test]
    fn inherit() {
        let node = NodeDescriptor {
            id: CanonicalId::mock(2),
            scope: ScopeId(vec![0]),
            kind: Kind::Value,
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"), None)),
        };
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = mock_state(
            &ctx,
            vec![(
                (ScopeId(vec![0]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(1)]),
            )],
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Integer))),
            )],
        );

        assert_eq!(
            super::infer(&state, &None, "foo", &node),
            Action::Infer(Type::Inherit(CanonicalId::mock(1)))
        );
    }

    #[test]
    fn not_found() {
        let node = NodeDescriptor {
            id: CanonicalId::mock(1),
            scope: ScopeId(vec![0]),
            kind: Kind::Value,
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"), None)),
        };
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = mock_state(&ctx, vec![], vec![]);

        assert_eq!(
            super::infer(&state, &None, "foo", &node),
            Action::Raise(Error::NotFound(str!("foo")))
        );
    }
}
