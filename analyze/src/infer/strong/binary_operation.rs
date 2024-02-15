use crate::{
    ast,
    infer::strong::{ExpectedType, SemanticError},
    strong::{Strong, StrongResult},
};
use kore::invariant;
use lang::{types, NodeId};

pub fn infer(
    op: &ast::BinaryOperator,
    lhs: &NodeId,
    rhs: &NodeId,
    result: &StrongResult,
) -> Option<Strong> {
    match op {
        ast::BinaryOperator::Add
        | ast::BinaryOperator::Subtract
        | ast::BinaryOperator::Multiply => match (
            result.as_strong(lhs, &types::RefKind::Value),
            result.as_strong(&rhs, &types::RefKind::Value),
        ) {
            (Some(Ok(types::Type::Integer)), Some(Ok(types::Type::Integer))) => {
                Some(Ok(types::Type::Integer))
            }

            (
                Some(Ok(types::Type::Integer | types::Type::Float)),
                Some(Ok(types::Type::Integer | types::Type::Float)),
            ) => Some(Ok(types::Type::Float)),

            (Some(Ok(types::Type::Integer | types::Type::Float)), Some(Ok(rhs_type))) => {
                Some(Err(SemanticError::UnexpectedShape(
                    (rhs_type.clone(), *rhs),
                    ExpectedType::Union(vec![types::Type::Integer, types::Type::Float]),
                )))
            }

            (Some(Ok(lhs_type)), Some(Ok(_))) => Some(Err(SemanticError::UnexpectedShape(
                (lhs_type.clone(), *lhs),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float]),
            ))),

            (Some(Err(_)), Some(Err(_))) => {
                Some(Err(SemanticError::NotInferrable(vec![*lhs, *rhs])))
            }
            (Some(Err(_)), _) => Some(Err(SemanticError::NotInferrable(vec![*lhs]))),
            (_, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![*rhs]))),

            (_, None) | (None, _) => None,
        },

        _ => {
            invariant!("all other binary operation types should be inferred during weak analysis")
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        infer::strong::{ExpectedType, SemanticError},
        test::fixture::strong_result_from,
    };
    use lang::{types, NodeId};

    #[test]
    fn none_result() {
        let result = strong_result_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer)))],
            vec![],
        );

        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1), &result),
            None
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0), &result),
            None
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(1), &result),
            None
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            None
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            None
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(1),
                &result
            ),
            None
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            None
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            None
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(1),
                &result
            ),
            None
        );
    }

    #[test]
    fn integer_result() {
        let result = strong_result_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer)))],
            vec![],
        );

        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(0), &result),
            Some(Ok(types::Type::Integer))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Integer))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Integer))
        );
    }

    #[test]
    fn float_result() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Float))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(0), &result),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1), &result),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0), &result),
            Some(Ok(types::Type::Float))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Float))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Ok(types::Type::Float))
        );
    }

    #[test]
    fn unexpected_shape() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1), &result),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0), &result),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let result = strong_result_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer))),
                (
                    NodeId(1),
                    (
                        types::RefKind::Value,
                        Err(SemanticError::NotInferrable(vec![])),
                    ),
                ),
            ],
            vec![],
        );

        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1), &result),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0), &result),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(1), &result),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Subtract,
                &NodeId(1),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );

        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(0),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(0),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            super::infer(
                &ast::BinaryOperator::Multiply,
                &NodeId(1),
                &NodeId(1),
                &result
            ),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );
    }
}
