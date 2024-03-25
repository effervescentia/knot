use super::partial;
use crate::{ast, error::ResolveError, strong};
use kore::invariant;
use lang::{
    types::{RefKind, ReferenceType, Type},
    NodeId,
};

const VALUE_KIND: RefKind = RefKind::Value;

pub fn infer<'a>(
    state: &strong::State,
    op: &ast::BinaryOperator,
    lhs: &NodeId,
    rhs: &NodeId,
) -> partial::Action<'a> {
    match op {
        ast::BinaryOperator::Add
        | ast::BinaryOperator::Subtract
        | ast::BinaryOperator::Multiply => match (
            state.resolve_type(lhs, &VALUE_KIND),
            state.resolve_type(rhs, &VALUE_KIND),
        ) {
            (Some(Ok(ReferenceType(Type::Integer))), Some(Ok(ReferenceType(Type::Integer)))) => {
                partial::Action::Infer(&Ok(strong::Type::Local(Type::Integer)))
            }

            (
                Some(Ok(ReferenceType(Type::Integer | Type::Float))),
                Some(Ok(ReferenceType(Type::Integer | Type::Float))),
            ) => partial::Action::Infer(&Ok(strong::Type::Local(Type::Float))),

            (Some(Err(_)), Some(Err(_))) => {
                partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*lhs, *rhs])))
            }

            (_, Some(Err(_))) | (Some(Ok(ReferenceType(Type::Integer | Type::Float))), _) => {
                partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*rhs])))
            }

            (Some(Err(_)), _) | (_, Some(Ok(ReferenceType(Type::Integer | Type::Float)))) => {
                partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![*lhs])))
            }

            (_, Some(_)) | (Some(_), _) => {
                partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![])))
            }

            (_, None) | (None, _) => partial::Action::Skip,
        },

        _ => {
            invariant!("all other binary operation types should be known after weak inference")
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast, error::ResolveError, infer::strong::partial, test::fixture::strong_state_from,
    };
    use lang::{
        types::{RefKind, ReferenceType, Type},
        NodeId,
    };

    #[derive(Debug, PartialEq)]
    struct MockNode;

    #[test]
    fn skip_infer() {
        let state = strong_state_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Ok(&ReferenceType(Type::Integer))),
            )],
            vec![],
        );
        let assert_skip =
            |op, lhs, rhs| assert_eq!(super::infer(&state, op, lhs, rhs), partial::Action::Skip);

        assert_skip(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1));
        assert_skip(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0));
        assert_skip(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(1));

        assert_skip(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(1));
        assert_skip(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(0));
        assert_skip(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(1));

        assert_skip(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(1));
        assert_skip(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(0));
        assert_skip(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(1));
    }

    #[test]
    fn integer_result() {
        let state = strong_state_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Ok(&ReferenceType(Type::Integer))),
            )],
            vec![],
        );
        let assert_integer = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&state, op, lhs, rhs),
                partial::Action::Infer(&Ok(&ReferenceType(Type::Integer)))
            )
        };

        assert_integer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(0));
        assert_integer(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(0));
        assert_integer(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(0));
    }

    #[test]
    fn float_result() {
        let state = strong_state_from(
            vec![],
            vec![
                (NodeId(0), (RefKind::Value, Ok(&ReferenceType(Type::Float)))),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(&ReferenceType(Type::Integer))),
                ),
            ],
            vec![],
        );
        let assert_float = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&state, op, lhs, rhs),
                partial::Action::Infer(&Ok(&ReferenceType(Type::Float)))
            )
        };

        assert_float(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(0));
        assert_float(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1));
        assert_float(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0));

        assert_float(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(0));
        assert_float(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(1));
        assert_float(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(0));

        assert_float(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(0));
        assert_float(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(1));
        assert_float(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(0));
    }

    // #[test]
    // fn unexpected_shape() {
    //     let state = strong_result_from(
    //         vec![],
    //         vec![
    //             (
    //                 NodeId(0),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
    //             ),
    //             (
    //                 NodeId(1),
    //                 (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
    //             ),
    //         ],
    //         vec![],
    //     );
    //     let assert_unexpected_shape = |op, lhs, rhs| {
    //         assert_eq!(
    //             super::infer(&state, op, lhs, rhs),
    //             partial::Action::Infer(&Err(SemanticError::UnexpectedShape(
    //                 (ShallowType(Type::Boolean), NodeId(1)),
    //                 ExpectedShape::Union(vec![Type::Integer, Type::Float])
    //             )))
    //         )
    //     };

    //     assert_unexpected_shape(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1));
    //     assert_unexpected_shape(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0));

    //     assert_unexpected_shape(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(1));
    //     assert_unexpected_shape(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(0));

    //     assert_unexpected_shape(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(1));
    //     assert_unexpected_shape(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(0));
    // }

    #[test]
    fn not_inferrable() {
        let state = strong_state_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (RefKind::Value, Ok(&ReferenceType(Type::Integer))),
                ),
                (
                    NodeId(1),
                    (RefKind::Value, Err(ResolveError::NotInferrable(vec![]))),
                ),
            ],
            vec![],
        );
        let assert_not_inferrable = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&state, op, lhs, rhs),
                partial::Action::Infer(&Err(ResolveError::NotInferrable(vec![NodeId(1)])))
            )
        };

        assert_not_inferrable(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1));
        assert_not_inferrable(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0));
        assert_not_inferrable(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(1));

        assert_not_inferrable(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(1));
        assert_not_inferrable(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(0));
        assert_not_inferrable(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(1));

        assert_not_inferrable(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(1));
        assert_not_inferrable(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(0));
        assert_not_inferrable(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(1));
    }
}
