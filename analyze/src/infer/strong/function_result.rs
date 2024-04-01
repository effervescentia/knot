use super::{
    data::{Action, Type},
    inherit,
    state::State,
};
use crate::error::ResolveError;
use lang::{
    types::{self, Enumerated, Kind},
    CanonicalId,
};

pub fn infer(state: &State, x: CanonicalId, kind: &Kind) -> Action {
    match state.resolve_any(&x) {
        Some(Ok(types::Type::Function(_, result))) => inherit::inherit(state, result, kind),

        Some(Ok(types::Type::Enumerated(Enumerated::Variant(_, instance)))) => Action::Infer(
            Type::Local(types::Type::Enumerated(Enumerated::Instance(instance))),
        ),

        Some(Err(_)) => Action::Raise(ResolveError::NotInferrable(vec![x])),

        // TODO: surface the underlying error (not a callable type)
        Some(Ok(_)) => Action::Raise(ResolveError::NotInferrable(vec![])),

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
        Context, ModuleMap,
    };
    use kore::assert_eq;
    use lang::{
        types::{self, Enumerated, Kind},
        CanonicalId, NodeId,
    };

    #[test]
    fn infer_function_result() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (
                        Kind::Value,
                        Ok(Type::Local(types::Type::Function(
                            vec![],
                            CanonicalId::mock(2),
                        ))),
                    ),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Local(types::Type::Integer))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Infer(Type::Inherit(CanonicalId::mock(2)))
        );
    }

    #[test]
    fn infer_enumerated_instance() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (
                        Kind::Value,
                        Ok(Type::Local(types::Type::Enumerated(Enumerated::Variant(
                            vec![],
                            CanonicalId::mock(2),
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
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Infer(Type::Local(types::Type::Enumerated(Enumerated::Instance(
                CanonicalId::mock(2)
            ))))
        );
    }

    #[test]
    fn skip() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(&ctx, vec![]);

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Skip
        );
    }

    #[test]
    fn not_inferrable() {
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Local(types::Type::Integer))),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Err(ResolveError::NotInferrable(vec![]))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1), &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![]))
        );
        assert_eq!(
            super::infer(&state, CanonicalId::mock(2), &Kind::Value),
            Action::Raise(ResolveError::NotInferrable(vec![CanonicalId::mock(2)]))
        );
    }
}
