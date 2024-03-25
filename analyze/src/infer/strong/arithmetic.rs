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
