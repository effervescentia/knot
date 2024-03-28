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

#[cfg(test)]
mod tests {
    use crate::{
        error::ResolveError,
        infer::strong::{
            data::{Action, Data},
            state::State,
        },
    };
    use kore::assert_eq;
    use lang::{
        types::{Enumerated, Kind, Type},
        NodeId,
    };

    #[test]
    fn infer_function_result() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Data::Local(Type::Function(vec![], NodeId(2)))),
                ),
            ),
            (NodeId(2), (Kind::Value, Ok(Data::Local(Type::Integer)))),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), &Kind::Value),
            Action::Infer(Data::Inherit(NodeId(2)))
        );
    }

    #[test]
    fn infer_enumerated_instance() {
        let state = State::from_types(vec![
            (
                NodeId(1),
                (
                    Kind::Value,
                    Ok(Data::Local(Type::Enumerated(Enumerated::Variant(
                        vec![],
                        NodeId(2),
                    )))),
                ),
            ),
            (
                NodeId(2),
                (
                    Kind::Value,
                    Ok(Data::Local(Type::Enumerated(Enumerated::Declaration(
                        vec![],
                    )))),
                ),
            ),
        ]);

        assert_eq!(
            super::infer(&state, NodeId(1), &Kind::Value),
            Action::Infer(Data::Local(Type::Enumerated(Enumerated::Instance(NodeId(
                2
            )))))
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
            (NodeId(1), (Kind::Value, Ok(Data::Local(Type::Integer)))),
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
