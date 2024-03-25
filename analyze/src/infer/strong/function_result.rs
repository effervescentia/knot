use super::{
    data::{Action, Data},
    inherit,
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{Enumerated, Kind, Type},
    NodeId,
};

pub fn infer(state: &State, x: NodeId, kind: &Kind) -> Action {
    match state.resolve_any(&x) {
        Some(Ok(Type::Function(_, result))) => inherit::inherit(state, *result, kind),

        Some(Ok(Type::Enumerated(Enumerated::Variant(_, instance)))) => Action::Infer(Data::Local(
            Type::Enumerated(Enumerated::Instance(*instance)),
        )),

        Some(Ok(_)) => Action::Raise(ResolveError::NotInferrable(vec![])),

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![x])),

        None => Action::Skip,
    }
}
