use super::partial;
use crate::{data::ScopedType, strong};
use lang::{types, NodeId};

pub fn inherit<'a, Node>(
    strong: &strong::Result,
    node: Node,
    from_id: &NodeId,
    from_kind: &types::RefKind,
) -> partial::Action<'a, Node> {
    match strong.get_shallow_type(from_id, from_kind) {
        Some(Ok(ScopedType::Inherit(next_from_id))) => {
            inherit(strong, node, next_from_id, from_kind)
        }

        Some(x) => partial::Action::Infer(node, &Ok(ScopedType::Inherit(*from_id))),

        None => partial::Action::SkipInherit(node, from_id),
    }
}
