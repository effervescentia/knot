use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::ResolveError;
use lang::{types, CanonicalId};

pub fn infer(state: &State, lhs: CanonicalId, rhs: CanonicalId) -> Action {
    match (state.resolve_value(&lhs), state.resolve_value(&rhs)) {
        (Some(Ok(types::Type::Integer)), Some(Ok(types::Type::Integer))) => {
            Action::Infer(Type::Value(types::Type::Integer))
        }

        (
            Some(Ok(types::Type::Integer | types::Type::Float)),
            Some(Ok(types::Type::Integer | types::Type::Float)),
        ) => Action::Infer(Type::Value(types::Type::Float)),

        (None, _) | (_, None) => Action::Skip,

        (Some(Err(_)), Some(Err(_))) => Action::Raise(ResolveError::NotInferrable(vec![lhs, rhs])),
        (Some(Err(_)), _) => Action::Raise(ResolveError::NotInferrable(vec![lhs])),
        (_, Some(Err(_))) => Action::Raise(ResolveError::NotInferrable(vec![rhs])),

        // TODO: surface the underlying error (types not valid for arithmetic)
        (Some(_), Some(_)) => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
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
    fn infer_integer() {
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
            super::infer(&state, CanonicalId::mock(1), CanonicalId::mock(1)),
            Action::Infer(Type::Value(types::Type::Integer))
        );
    }

    #[test]
    fn infer_float() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Value(types::Type::Float))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), CanonicalId::mock(2)),
            Action::Infer(Type::Value(types::Type::Float))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(2), CanonicalId::mock(1)),
            Action::Infer(Type::Value(types::Type::Float))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(2), CanonicalId::mock(2)),
            Action::Infer(Type::Value(types::Type::Float))
        );
    }

    #[test]
    fn skip() {
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
            super::infer(&state, CanonicalId::mock(1), CanonicalId::mock(3)),
            Action::Skip
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(3), CanonicalId::mock(1)),
            Action::Skip
        );
    }

    #[test]
    fn not_inferrable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Value(types::Type::Boolean))),
                ),
                (
                    NodeId(3),
                    (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), CanonicalId::mock(3)),
            Action::Raise(ResolveError::NotInferrable(vec![CanonicalId::mock(3)]))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(3), CanonicalId::mock(1)),
            Action::Raise(ResolveError::NotInferrable(vec![CanonicalId::mock(3)]))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(3), CanonicalId::mock(3)),
            Action::Raise(ResolveError::NotInferrable(vec![
                CanonicalId::mock(3),
                CanonicalId::mock(3)
            ]))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(2), CanonicalId::mock(2)),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }
}
