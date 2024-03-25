use super::{data::Action, inherit, state::State};
use crate::{error::ResolveError, infer::NodeDescriptor};

pub fn infer(state: &State, name: &str, node: &NodeDescriptor) -> Action {
    match state.bindings.resolve(node, name) {
        Some(from_id) => inherit::inherit(state, from_id, &node.kind),

        None => Action::Raise(ResolveError::NotFound(name.to_owned(), node.id)),
    }
}
