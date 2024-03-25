use super::partial;
use crate::{data::ScopedType, strong};
use lang::{types::RefKind, NodeId};

pub fn inherit<'a>(
    strong: &strong::Result,
    from_id: &NodeId,
    from_kind: &RefKind,
) -> partial::Action<'a> {
    match strong.get_type(from_id, from_kind) {
        Some(Ok(ScopedType::Inherit(next_from_id))) => inherit(strong, next_from_id, from_kind),

        Some(x) => partial::Action::Infer(&Ok(ScopedType::Inherit(*from_id))),

        None => partial::Action::SkipInherit(from_id),
    }
}
