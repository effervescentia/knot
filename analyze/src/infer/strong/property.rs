use super::{
    data::{Action, Data},
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{Enumerated, Kind, Type},
    NodeId,
};

fn infer_module(
    declarations: &[(String, Kind, NodeId)],
    property: &str,
    allowed_kind: &Kind,
) -> Action {
    match declarations.iter().find(|(name, ..)| name == property) {
        // TODO: `inherit::inherit`
        Some((_, kind, id)) if allowed_kind.can_accept(kind) => Action::Infer(Data::Inherit(*id)),

        Some(_) | None => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

fn infer_enumerated(
    variants: &[(String, Vec<NodeId>)],
    property: &str,
    enumerated: &NodeId,
) -> Action {
    match variants.iter().find(|(name, _)| name == property) {
        Some((_, parameters)) => Action::Infer(Data::Local(Type::Enumerated(Enumerated::Variant(
            parameters.clone(),
            *enumerated,
        )))),

        None => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

pub fn infer(state: &State, lhs: &NodeId, property: &str, allowed_kind: &Kind) -> Action {
    match state.resolve_any(lhs) {
        Some(Ok(x)) => match x {
            Type::Module(declarations) => infer_module(declarations, property, allowed_kind),

            Type::Enumerated(Enumerated::Declaration(variants)) => {
                infer_enumerated(variants, property, lhs)
            }

            _ => Action::Raise(ResolveError::NotInferrable(vec![])),
        },

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![*lhs])),

        None => Action::Skip,
    }
}
