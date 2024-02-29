use super::partial;
use crate::strong;
use lang::{types::RefKind, NodeId};

pub fn inherit<'a>(
    state: &strong::State,
    from_id: &NodeId,
    from_kind: &RefKind,
) -> partial::Action<'a> {
    match state.get_type(from_id, from_kind) {
        Some(Ok(ScopedType::Inherit(next_from_id))) => inherit(state, next_from_id, from_kind),

        Some(x) => partial::Action::Infer(&Ok(ScopedType::Inherit(*from_id))),

        None => partial::Action::Inherit(from_id),
    }
}
