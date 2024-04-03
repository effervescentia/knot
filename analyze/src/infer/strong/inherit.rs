use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::Error;
use lang::{types::Kind, CanonicalId};

pub fn inherit(state: &State, from_id: CanonicalId, from_kind: &Kind) -> Action {
    if !state.is_local(&from_id) {
        return Action::Infer(Type::Inherit(from_id));
    }

    match state.get_type(&from_id.1, from_kind) {
        Some(Ok(Type::Inherit(next_from_id))) => inherit_any(state, *next_from_id),

        Some(Ok(_)) => Action::Infer(Type::Inherit(from_id)),

        Some(Err(_)) => Action::Raise(Error::NotInferrable(vec![from_id])),

        None => Action::InheritAndSkip(from_id),
    }
}

pub fn inherit_any(state: &State, from_id: CanonicalId) -> Action {
    inherit(state, from_id, &Kind::Mixed)
}

#[cfg(test)]
mod tests {
    use crate::{
        error::Error,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
        Context, ModuleMap,
    };
    use kore::assert_eq;
    use lang::{
        types::{self, Kind},
        CanonicalId, NodeId,
    };

    #[test]
    fn inherit_and_skip() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(&ctx, vec![]);

        assert_eq!(
            super::inherit(&state, CanonicalId::mock(1), &Kind::Value),
            Action::InheritAndSkip(CanonicalId::mock(1))
        );
    }

    #[test]
    fn inherit() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Integer))),
            )],
        );

        assert_eq!(
            super::inherit(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Infer(Type::Inherit(CanonicalId::mock(1)))
        );
    }

    #[test]
    fn recursive_inherit() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(2)))),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(3)))),
                ),
                (
                    NodeId(3),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
            ],
        );

        assert_eq!(
            super::inherit(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Infer(Type::Inherit(CanonicalId::mock(3)))
        );
    }

    #[test]
    fn not_inferrable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![(NodeId(1), (Kind::Value, Err(Error::NotInferrable(vec![]))))],
        );

        assert_eq!(
            super::inherit(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(1)]))
        );
    }
}
