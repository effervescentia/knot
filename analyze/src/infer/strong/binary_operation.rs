use crate::{
    ast,
    infer::strong::{ExpectedType, SemanticError, Strong},
    strong::StrongResult,
};
use kore::invariant;
use lang::{types, NodeId};

pub fn infer(
    op: &ast::BinaryOperator,
    lhs: NodeId,
    rhs: NodeId,
    ctx: &StrongResult,
) -> Option<Strong> {
    match op {
        ast::BinaryOperator::Add
        | ast::BinaryOperator::Subtract
        | ast::BinaryOperator::Multiply => match (
            ctx.as_strong(&lhs, &types::RefKind::Value),
            ctx.as_strong(&rhs, &types::RefKind::Value),
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
                    (rhs_type.clone(), rhs),
                    ExpectedType::Union(vec![types::Type::Integer, types::Type::Float]),
                )))
            }

            (Some(Ok(lhs_type)), Some(Ok(_))) => Some(Err(SemanticError::UnexpectedShape(
                (lhs_type.clone(), lhs),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float]),
            ))),

            (Some(Err(_)), Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![lhs, rhs]))),
            (Some(Err(_)), _) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),
            (_, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![rhs]))),

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
        infer::strong::{ExpectedType, SemanticError, Strong},
        strong::StrongResult,
        test::fixture::strong_ctx_from,
    };
    use lang::{types, NodeId};

    fn infer(
        op: ast::BinaryOperator,
        lhs: usize,
        rhs: usize,
        ctx: &StrongResult,
    ) -> Option<Strong> {
        super::infer(&op, NodeId(lhs), NodeId(rhs), ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer)))],
            vec![],
        );

        assert_eq!(infer(ast::BinaryOperator::Add, 0, 1, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Add, 1, 0, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Add, 1, 1, &ctx), None);

        assert_eq!(infer(ast::BinaryOperator::Subtract, 0, 1, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Subtract, 1, 0, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Subtract, 1, 1, &ctx), None);

        assert_eq!(infer(ast::BinaryOperator::Multiply, 0, 1, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Multiply, 1, 0, &ctx), None);
        assert_eq!(infer(ast::BinaryOperator::Multiply, 1, 1, &ctx), None);
    }

    #[test]
    fn integer_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer)))],
            vec![],
        );

        assert_eq!(
            infer(ast::BinaryOperator::Add, 0, 0, &ctx),
            Some(Ok(types::Type::Integer))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 0, 0, &ctx),
            Some(Ok(types::Type::Integer))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 0, 0, &ctx),
            Some(Ok(types::Type::Integer))
        );
    }

    #[test]
    fn float_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Float))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            infer(ast::BinaryOperator::Add, 0, 0, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Add, 0, 1, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Add, 1, 0, &ctx),
            Some(Ok(types::Type::Float))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 0, 0, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Ok(types::Type::Float))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 0, 0, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Ok(types::Type::Float))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Ok(types::Type::Float))
        );
    }

    #[test]
    fn unexpected_shape() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Integer))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Boolean))),
            ],
            vec![],
        );

        assert_eq!(
            infer(ast::BinaryOperator::Add, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Add, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (types::Type::Boolean, NodeId(1)),
                ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let ctx = strong_ctx_from(
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
            infer(ast::BinaryOperator::Add, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Add, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Add, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Subtract, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );

        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![NodeId(1)])))
        );
        assert_eq!(
            infer(ast::BinaryOperator::Multiply, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![
                NodeId(1),
                NodeId(1)
            ])))
        );
    }
}
