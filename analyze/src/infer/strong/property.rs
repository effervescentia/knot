use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{self, Enumerated, Kind},
    NodeId,
};

fn infer_module(
    declarations: &[(String, Kind, NodeId)],
    property: &str,
    allowed_kind: &Kind,
) -> Action {
    match declarations.iter().find(|(name, ..)| name == property) {
        // TODO: `inherit::inherit`
        Some((_, kind, id)) if allowed_kind.can_accept(kind) => Action::Infer(Type::Inherit(*id)),

        Some(_) | None => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

fn infer_enumerated(
    variants: &[(String, Vec<NodeId>)],
    property: &str,
    enumerated: &NodeId,
) -> Action {
    match variants.iter().find(|(name, _)| name == property) {
        Some((_, parameters)) => Action::Infer(Type::Local(types::Type::Enumerated(
            Enumerated::Variant(parameters.clone(), *enumerated),
        ))),

        None => Action::Raise(ResolveError::NotInferrable(vec![])),
    }
}

pub fn infer(state: &State, lhs: NodeId, property: &str, allowed_kind: &Kind) -> Action {
    match state.resolve_any(&lhs) {
        Some(Ok(x)) => match x {
            Type::Module(declarations) => infer_module(declarations, property, allowed_kind),

            Type::Enumerated(Enumerated::Declaration(variants)) => {
                infer_enumerated(variants, property, &lhs)
            }

            _ => Action::Raise(ResolveError::NotInferrable(vec![])),
        },

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![lhs])),

        None => Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Enumerated, Kind},
        NodeId,
    };

    #[test]
    fn infer_enum_variant() {
        let state = State::from_types(vec![(
            NodeId(1),
            (
                Kind::Value,
                Ok(Type::Local(types::Type::Enumerated(
                    Enumerated::Declaration(vec![(str!("foo"), vec![NodeId(2), NodeId(3)])]),
                ))),
            ),
        )]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Infer(Type::Local(types::Type::Enumerated(Enumerated::Variant(
                vec![NodeId(2), NodeId(3)],
                NodeId(1)
            ))))
        );
    }

    #[test]
    fn infer_enum_variant_not_inferrable() {
        let state = State::from_types(vec![(
            NodeId(1),
            (
                Kind::Value,
                Ok(Type::Local(types::Type::Enumerated(
                    Enumerated::Declaration(vec![]),
                ))),
            ),
        )]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }

    #[test]
    fn infer_module_entry() {
        let state = State::from_types(vec![(
            NodeId(1),
            (
                Kind::Value,
                Ok(Type::Local(types::Type::Module(vec![(
                    str!("foo"),
                    Kind::Value,
                    NodeId(2),
                )]))),
            ),
        )]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Infer(Type::Inherit(NodeId(2)))
        );
    }

    #[test]
    fn infer_module_entry_not_inferrable() {
        let state = State::from_types(vec![(
            NodeId(1),
            (
                Kind::Value,
                Ok(Type::Local(types::Type::Module(vec![(
                    str!("foo"),
                    Kind::Type,
                    NodeId(2),
                )]))),
            ),
        )]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
        assert_eq!(
            super::infer(&state, NodeId(1), "bar", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }

    #[test]
    fn skip() {
        let state = State::from_types(vec![]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Skip
        );
    }

    #[test]
    fn not_inferrable() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
            ),
            (
                NodeId(2),
                (Kind::Value, Ok(Type::Local(types::Type::String))),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(1)]))
        );
        assert_eq!(
            super::infer(&state, NodeId(2), "foo", &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
    }
}
