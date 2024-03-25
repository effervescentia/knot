use super::{
    data::{Action, Data},
    state::State,
};
use crate::error::ResolveError;
use lang::{types::Kind, NodeId};

pub fn inherit(state: &State, from_id: NodeId, from_kind: &Kind) -> Action {
    match state.get_type(&from_id, from_kind) {
        Some(Ok(Data::Inherit(next_from_id))) => inherit(state, *next_from_id, from_kind),

        Some(Ok(_)) => Action::Infer(Data::Inherit(from_id)),

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![from_id])),

        None => Action::Inherit(from_id),
    }
}
