use super::{
    data::{Action, Data},
    state::State,
};
use crate::error::ResolveError;
use lang::{types::Type, NodeId};

pub fn infer(state: &State, lhs: NodeId, rhs: NodeId) -> Action {
    match (state.resolve_value(&lhs), state.resolve_value(&rhs)) {
        (Some(Ok(Type::Integer)), Some(Ok(Type::Integer))) => {
            Action::Infer(Data::Local(Type::Integer))
        }

        (Some(Ok(Type::Integer | Type::Float)), Some(Ok(Type::Integer | Type::Float))) => {
            Action::Infer(Data::Local(Type::Float))
        }

        (None, _) | (_, None) => Action::Skip,

        (Some(Err(_)), Some(Err(_))) => Action::Raise(ResolveError::NotInferrable(vec![lhs, rhs])),
        (Some(Err(_)), _) => Action::Raise(ResolveError::NotInferrable(vec![lhs])),
        (_, Some(Err(_))) => Action::Raise(ResolveError::NotInferrable(vec![rhs])),
        (Some(_), Some(_)) => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::strong::{
            data::{Action, Data},
            state::State,
        },
    };
    use kore::assert_eq;
    use lang::{
        types::{Kind, Type},
        NodeId,
    };

    #[test]
    fn infer_integer() {
        let state = State::from_types(vec![(
            NodeId(1),
            (Kind::Value, Ok(Data::Local(Type::Integer))),
        )]);

        assert_eq!(
            super::infer(&state, NodeId(1), NodeId(1)),
            Action::Infer(Data::Local(Type::Integer))
        );
    }

    #[test]
    fn infer_float() {
        let state = State::from_types(vec![
            (NodeId(1), (Kind::Value, Ok(Data::Local(Type::Integer)))),
            (NodeId(2), (Kind::Value, Ok(Data::Local(Type::Float)))),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), NodeId(2)),
            Action::Infer(Data::Local(Type::Float))
        );
        assert_eq!(
            super::infer(&state, NodeId(2), NodeId(1)),
            Action::Infer(Data::Local(Type::Float))
        );
        assert_eq!(
            super::infer(&state, NodeId(2), NodeId(2)),
            Action::Infer(Data::Local(Type::Float))
        );
    }

    #[test]
    fn skip() {
        let state = State::from_types(vec![(
            NodeId(1),
            (Kind::Value, Ok(Data::Local(Type::Integer))),
        )]);

        assert_eq!(super::infer(&state, NodeId(1), NodeId(3)), Action::Skip);
        assert_eq!(super::infer(&state, NodeId(3), NodeId(1)), Action::Skip);
    }

    #[test]
    fn not_inferrable() {
        let state = State::from_types(vec![
            (NodeId(1), (Kind::Value, Ok(Data::Local(Type::Integer)))),
            (NodeId(2), (Kind::Value, Ok(Data::Local(Type::Boolean)))),
            (
                NodeId(3),
                (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), NodeId(3)),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(3)]))
        );
        assert_eq!(
            super::infer(&state, NodeId(3), NodeId(1)),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(3)]))
        );
        assert_eq!(
            super::infer(&state, NodeId(3), NodeId(3)),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(3), NodeId(3)]))
        );
        assert_eq!(
            super::infer(&state, NodeId(2), NodeId(2)),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }
}
