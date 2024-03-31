use super::{
    data::{Action, Type},
    inherit,
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{self, Enumerated, Kind},
    NodeId,
};

pub fn infer(state: &State, x: NodeId, kind: &Kind) -> Action {
    match state.resolve_any(&x) {
        Some(Ok(Type::Function(_, result))) => inherit::inherit(state, *result, kind),

        Some(Ok(Type::Enumerated(Enumerated::Variant(_, instance)))) => Action::Infer(Type::Local(
            types::Type::Enumerated(Enumerated::Instance(*instance)),
        )),

        Some(Ok(_)) => Action::Raise(ResolveError::NotInferrable(vec![])),
        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![x])),

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
    use kore::assert_eq;
    use lang::{
        types::{self, Enumerated, Kind},
        NodeId,
    };

    #[test]
    fn infer_function_result() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Local(types::Type::Function(vec![], NodeId(2)))),
                ),
            ),
            (
                NodeId(2),
                (Kind::Value, Ok(Type::Local(types::Type::Integer))),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), &Kind::Value),
            Action::Infer(Type::Inherit(NodeId(2)))
        );
    }

    #[test]
    fn infer_enumerated_instance() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Type::Local(types::Type::Enumerated(Enumerated::Variant(
                        vec![],
                        NodeId(2),
                    )))),
                ),
            ),
            (
                NodeId(2),
                (
                    Kind::Value,
                    Ok(Type::Local(types::Type::Enumerated(
                        Enumerated::Declaration(vec![]),
                    ))),
                ),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), &Kind::Value),
            Action::Infer(Type::Local(types::Type::Enumerated(Enumerated::Instance(
                NodeId(2)
            ))))
        );
    }

    #[test]
    fn skip() {
        let state = State::from_types(vec![]);

        assert_eq!(super::infer(&state, NodeId(1), &Kind::Value), Action::Skip);
    }

    #[test]
    fn not_inferrable() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (Kind::Value, Ok(Type::Local(types::Type::Integer))),
            ),
            (
                NodeId(2),
                (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
        assert_eq!(
            super::infer(&state, NodeId(2), &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![NodeId(2)]))
        );
    }
}
