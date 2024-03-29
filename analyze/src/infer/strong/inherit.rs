use super::{
    data::{Action, Data},
    state::State,
};
use crate::error::ResolveError;
use lang::{types::Kind, NodeId};

pub fn inherit(state: &State, from_id: NodeId, from_kind: &Kind) -> Action {
    match state.get_type(&from_id, from_kind) {
        Some(Ok(Data::Inherit(next_from_id))) => inherit(state, *next_from_id, &Kind::Mixed),

        Some(Ok(_)) => Action::Infer(Data::Inherit(from_id)),

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![from_id])),

        None => Action::InheritAndSkip(from_id),
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
    fn inherit_and_skip() {
        let state = State::from_types(vec![]);

        assert_eq!(
            super::inherit(&state, NodeId(1), &Kind::Value),
            Action::InheritAndSkip(NodeId(1))
        );
    }

    #[test]
    fn inherit() {
        let state = State::from_types(vec![(
            NodeId(1),
            (Kind::Value, Ok(Data::Local(Type::Integer))),
        )]);

        assert_eq!(
            super::inherit(&state, NodeId(1), &Kind::Value),
            Action::Infer(Data::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn recursive_inherit() {
        let state = State::from_types(vec![
            (NodeId(1), (Kind::Value, Ok(Data::Inherit(NodeId(2))))),
            (NodeId(2), (Kind::Value, Ok(Data::Inherit(NodeId(3))))),
            (NodeId(3), (Kind::Value, Ok(Data::Local(Type::Integer)))),
        ]);

        assert_eq!(
            super::inherit(&state, NodeId(1), &Kind::Value),
            Action::Infer(Data::Inherit(NodeId(3)))
        );
    }

    #[test]
    fn not_inferrable() {
        let state = State::from_types(vec![(
            NodeId(1),
            (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
        )]);

        assert_eq!(
            super::inherit(&state, NodeId(1), &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(1)]))
        );
    }
}
