use super::{
    data::{Action, Type},
    inherit,
    state::State,
};
use crate::error::Error;
use lang::{
    types::{self, Enumerated},
    CanonicalId,
};

pub fn infer(state: &State, x: CanonicalId) -> Action {
    match state.resolve_any(&x) {
        Some(Ok(types::Type::Function(_, result))) => inherit::inherit_any(state, result),

        Some(Ok(types::Type::Enumerated(
            enum_name,
            Enumerated::Variant(variant_name, _, instance),
        ))) => Action::Infer(Type::Value(types::Type::Enumerated(
            enum_name,
            Enumerated::Instance(variant_name, instance),
        ))),

        Some(Ok(_)) => Action::Raise(Error::NotCallable(x)),

        Some(Err(_)) => Action::Raise(Error::NotInferrable(vec![x])),

        None => Action::Skip,
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyze_mock,
        error::Error,
        infer::strong::{
            data::{Action, Type},
            state::State,
        },
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{self, Enumerated, Kind},
        CanonicalId, NodeId,
    };

    #[test]
    fn infer_function_result() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (
                        Kind::Value,
                        Ok(Type::Value(types::Type::Function(
                            vec![],
                            CanonicalId::mock(2),
                        ))),
                    ),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1)),
            Action::Infer(Type::Inherit(CanonicalId::mock(2)))
        );
    }

    #[test]
    fn infer_enumerated_instance() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (
                        Kind::Value,
                        Ok(Type::Value(types::Type::Enumerated(
                            str!("Foo"),
                            Enumerated::Variant(str!("Bar"), vec![], CanonicalId::mock(2)),
                        ))),
                    ),
                ),
                (
                    NodeId(2),
                    (
                        Kind::Value,
                        Ok(Type::Value(types::Type::Enumerated(
                            str!("Foo"),
                            Enumerated::Declaration(vec![]),
                        ))),
                    ),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1)),
            Action::Infer(Type::Value(types::Type::Enumerated(
                str!("Foo"),
                Enumerated::Instance(str!("Bar"), CanonicalId::mock(2))
            )))
        );
    }

    #[test]
    fn skip() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(&ctx, vec![]);

        assert_eq!(super::infer(&state, CanonicalId::mock(1)), Action::Skip);
    }

    #[test]
    fn not_callable() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Integer))),
            )],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1)),
            Action::Raise(Error::NotCallable(CanonicalId::mock(1)))
        );
    }

    #[test]
    fn not_inferrable() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(NodeId(1), (Kind::Value, Err(Error::NotInferrable(vec![]))))],
        );

        assert_eq!(
            super::infer(&state, CanonicalId::mock(1)),
            Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(1)]))
        );
    }
}
