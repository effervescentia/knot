use crate::{
    context::StrongContext,
    infer::strong::{ExpectedType, SemanticError, Strong},
    RefKind, Type,
};
use lang::ast::BinaryOperator;

pub fn infer(op: &BinaryOperator, lhs: usize, rhs: usize, ctx: &StrongContext) -> Option<Strong> {
    match op {
        BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => match (
            ctx.get_strong(&lhs, &RefKind::Value),
            ctx.get_strong(&rhs, &RefKind::Value),
        ) {
            (Some(Ok(Type::Integer)), Some(Ok(Type::Integer))) => Some(Ok(Type::Integer)),

            (Some(Ok(Type::Integer | Type::Float)), Some(Ok(Type::Integer | Type::Float))) => {
                Some(Ok(Type::Float))
            }

            (Some(Ok(Type::Integer | Type::Float)), Some(Ok(rhs_type))) => {
                Some(Err(SemanticError::UnexpectedShape(
                    (rhs_type.clone(), rhs),
                    ExpectedType::Union(vec![Type::Integer, Type::Float]),
                )))
            }

            (Some(Ok(lhs_type)), Some(Ok(_))) => Some(Err(SemanticError::UnexpectedShape(
                (lhs_type.clone(), lhs),
                ExpectedType::Union(vec![Type::Integer, Type::Float]),
            ))),

            (Some(Err(_)), Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![lhs, rhs]))),
            (Some(Err(_)), _) => Some(Err(SemanticError::NotInferrable(vec![lhs]))),
            (_, Some(Err(_))) => Some(Err(SemanticError::NotInferrable(vec![rhs]))),

            (_, None) | (None, _) => None,
        },

        _ => {
            unreachable!("all other binary operation types should be inferred during weak analysis")
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::StrongContext,
        infer::strong::{ExpectedType, SemanticError, Strong},
        test::fixture::strong_ctx_from,
        types::Type,
        RefKind,
    };
    use lang::ast::BinaryOperator;

    fn infer(op: BinaryOperator, lhs: usize, rhs: usize, ctx: &StrongContext) -> Option<Strong> {
        super::infer(&op, lhs, rhs, &ctx)
    }

    #[test]
    fn none_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(0, (RefKind::Value, Ok(Type::Integer)))],
            vec![],
        );

        assert_eq!(infer(BinaryOperator::Add, 0, 1, &ctx), None);
        assert_eq!(infer(BinaryOperator::Add, 1, 0, &ctx), None);
        assert_eq!(infer(BinaryOperator::Add, 1, 1, &ctx), None);

        assert_eq!(infer(BinaryOperator::Subtract, 0, 1, &ctx), None);
        assert_eq!(infer(BinaryOperator::Subtract, 1, 0, &ctx), None);
        assert_eq!(infer(BinaryOperator::Subtract, 1, 1, &ctx), None);

        assert_eq!(infer(BinaryOperator::Multiply, 0, 1, &ctx), None);
        assert_eq!(infer(BinaryOperator::Multiply, 1, 0, &ctx), None);
        assert_eq!(infer(BinaryOperator::Multiply, 1, 1, &ctx), None);
    }

    #[test]
    fn integer_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![(0, (RefKind::Value, Ok(Type::Integer)))],
            vec![],
        );

        assert_eq!(
            infer(BinaryOperator::Add, 0, 0, &ctx),
            Some(Ok(Type::Integer))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 0, 0, &ctx),
            Some(Ok(Type::Integer))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 0, 0, &ctx),
            Some(Ok(Type::Integer))
        );
    }

    #[test]
    fn float_result() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Float))),
                (1, (RefKind::Value, Ok(Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            infer(BinaryOperator::Add, 0, 0, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Add, 0, 1, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Add, 1, 0, &ctx),
            Some(Ok(Type::Float))
        );

        assert_eq!(
            infer(BinaryOperator::Subtract, 0, 0, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Ok(Type::Float))
        );

        assert_eq!(
            infer(BinaryOperator::Multiply, 0, 0, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Ok(Type::Float))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Ok(Type::Float))
        );
    }

    #[test]
    fn unexpected_shape() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Integer))),
                (1, (RefKind::Value, Ok(Type::Boolean))),
            ],
            vec![],
        );

        assert_eq!(
            infer(BinaryOperator::Add, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );
        assert_eq!(
            infer(BinaryOperator::Add, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );

        assert_eq!(
            infer(BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );

        assert_eq!(
            infer(BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Err(SemanticError::UnexpectedShape(
                (Type::Boolean, 1),
                ExpectedType::Union(vec![Type::Integer, Type::Float])
            )))
        );
    }

    #[test]
    fn not_inferrable() {
        let ctx = strong_ctx_from(
            vec![],
            vec![
                (0, (RefKind::Value, Ok(Type::Integer))),
                (
                    1,
                    (RefKind::Value, Err(SemanticError::NotInferrable(vec![]))),
                ),
            ],
            vec![],
        );

        assert_eq!(
            infer(BinaryOperator::Add, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Add, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Add, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1, 1])))
        );

        assert_eq!(
            infer(BinaryOperator::Subtract, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Subtract, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1, 1])))
        );

        assert_eq!(
            infer(BinaryOperator::Multiply, 0, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 1, 0, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1])))
        );
        assert_eq!(
            infer(BinaryOperator::Multiply, 1, 1, &ctx),
            Some(Err(SemanticError::NotInferrable(vec![1, 1])))
        );
    }
}
