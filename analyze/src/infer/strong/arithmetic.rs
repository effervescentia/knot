use super::{
    data::{Action, Type},
    state::State,
};
use crate::error::Error;
use lang::{ast, types, CanonicalId};

pub fn infer(state: &State, op: ast::BinaryOperator, lhs: CanonicalId, rhs: CanonicalId) -> Action {
    match (state.resolve_value(&lhs), state.resolve_value(&rhs)) {
        (Some(Ok(types::Type::Integer)), Some(Ok(types::Type::Integer))) => {
            Action::Infer(Type::Value(types::Type::Integer))
        }

        (
            Some(Ok(types::Type::Integer | types::Type::Float)),
            Some(Ok(types::Type::Integer | types::Type::Float)),
        ) => Action::Infer(Type::Value(types::Type::Float)),

        (None, _) | (_, None) => Action::Skip,

        (Some(Err(_)), Some(Err(_))) => Action::Raise(Error::NotInferrable(vec![lhs, rhs])),
        (Some(Err(_)), _) => Action::Raise(Error::NotInferrable(vec![lhs])),
        (_, Some(Err(_))) => Action::Raise(Error::NotInferrable(vec![rhs])),

        (Some(_), Some(_)) => Action::Raise(Error::BinaryOperationNotSupported(op, lhs, rhs)),
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
    use kore::assert_eq;
    use lang::{
        ast::{self, BinaryOperator},
        types::{self, Kind},
        CanonicalId, NodeId,
    };

    const OP: ast::BinaryOperator = BinaryOperator::Add;

    #[test]
    fn infer_integer() {
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
            super::infer(&state, OP, CanonicalId::mock(1), CanonicalId::mock(1)),
            Action::Infer(Type::Value(types::Type::Integer))
        );
    }

    #[test]
    fn infer_float() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
                (
                    NodeId(2),
                    (Kind::Value, Ok(Type::Value(types::Type::Float))),
                ),
            ],
        );

        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(1), CanonicalId::mock(2)),
            Action::Infer(Type::Value(types::Type::Float))
        );
        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(2), CanonicalId::mock(1)),
            Action::Infer(Type::Value(types::Type::Float))
        );
        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(2), CanonicalId::mock(2)),
            Action::Infer(Type::Value(types::Type::Float))
        );
    }

    #[test]
    fn skip() {
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
            super::infer(&state, OP, CanonicalId::mock(1), CanonicalId::mock(3)),
            Action::Skip
        );
        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(3), CanonicalId::mock(1)),
            Action::Skip
        );
    }

    #[test]
    fn operation_not_supported() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![(
                NodeId(1),
                (Kind::Value, Ok(Type::Value(types::Type::Boolean))),
            )],
        );

        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(1), CanonicalId::mock(1)),
            Action::Raise(Error::BinaryOperationNotSupported(
                OP,
                CanonicalId::mock(1),
                CanonicalId::mock(1)
            ))
        );
    }

    #[test]
    fn not_inferrable() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let state = State::from_types(
            &ctx,
            vec![
                (
                    NodeId(1),
                    (Kind::Value, Ok(Type::Value(types::Type::Integer))),
                ),
                (NodeId(2), (Kind::Value, Err(Error::NotInferrable(vec![])))),
            ],
        );

        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(1), CanonicalId::mock(2)),
            Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(2)]))
        );
        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(2), CanonicalId::mock(1)),
            Action::Raise(Error::NotInferrable(vec![CanonicalId::mock(2)]))
        );
        assert_eq!(
            super::infer(&state, OP, CanonicalId::mock(2), CanonicalId::mock(2)),
            Action::Raise(Error::NotInferrable(vec![
                CanonicalId::mock(2),
                CanonicalId::mock(2)
            ]))
        );
    }
}
