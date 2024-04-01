use lang::Canonicalize;

use super::{data::Action, inherit, state::State};
use crate::{error::ResolveError, infer::NodeDescriptor};

pub fn infer(state: &State, name: &str, node: &NodeDescriptor) -> Action {
    println!(
        "RESOLVING INFERENCE {name} {node:?} {:?}",
        state.bindings.resolve(node, name)
    );
    match state.bindings.resolve(node, name) {
        Some(from_id) => inherit::inherit(state, state.canonicalize(from_id), &node.kind),

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
                state::State,
            },
            weak, BindingMap, NodeDescriptor,
        },
        Context, ModuleMap,
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
        types: Vec<(NodeId, (Kind, Result<Type, ResolveError>))>,
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
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"))),
        };
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = mock_state(
            &ctx,
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
            Action::Infer(Type::Inherit(CanonicalId::mock(1)))
        );
    }

    #[test]
    fn not_found() {
        let node = NodeDescriptor {
            id: CanonicalId::mock(1),
            scope: ScopeId(vec![0]),
            kind: Kind::Value,
            weak: weak::Type::Infer(weak::Inference::Reference(str!("foo"))),
        };
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = mock_state(&ctx, vec![], vec![]);

        assert_eq!(
            super::infer(&state, "foo", &node),
            Action::Raise(ResolveError::NotFound(str!("foo"), CanonicalId::mock(1)))
        );
    }
}
