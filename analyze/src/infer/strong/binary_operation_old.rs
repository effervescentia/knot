use super::partial;
use crate::{
    ast,
    data::ScopedType,
    error::{ExpectedShape, SemanticError},
    strong,
};
use kore::invariant;
use lang::{
    types::{RefKind, ShallowType, Type},
    NodeId,
};

const VALUE_KIND: RefKind = RefKind::Value;

pub fn infer<'a>(
    strong: &strong::Result,
    op: &ast::BinaryOperator,
    lhs: &NodeId,
    rhs: &NodeId,
) -> partial::Action<'a> {
    match op {
        ast::BinaryOperator::Add
        | ast::BinaryOperator::Subtract
        | ast::BinaryOperator::Multiply => match (
            strong.get_shallow_type(lhs, &VALUE_KIND),
            strong.get_shallow_type(&rhs, &VALUE_KIND),
        ) {
            (Some(Ok(ShallowType(Type::Integer))), Some(Ok(ShallowType(Type::Integer)))) => {
                partial::Action::Infer(&Ok(ScopedType::Type(Type::Integer)))
            }

            (
                Some(Ok(ShallowType(Type::Integer | Type::Float))),
                Some(Ok(ShallowType(Type::Integer | Type::Float))),
            ) => partial::Action::Infer(&Ok(ScopedType::Type(Type::Float))),

            (Some(Ok(ShallowType(Type::Integer | Type::Float))), Some(Ok(rhs_type))) => {
                partial::Action::Infer(&Err(SemanticError::UnexpectedShape(
                    (rhs_type.clone(), *rhs),
                    ExpectedShape::Union(vec![Type::Integer, Type::Float]),
                )))
            }

            (Some(Ok(lhs_type)), Some(_)) => {
                partial::Action::Infer(&Err(SemanticError::UnexpectedShape(
                    (lhs_type.clone(), *lhs),
                    ExpectedShape::Union(vec![Type::Integer, Type::Float]),
                )))
            }

            (Some(Err(_)), Some(Err(_))) => {
                partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![*lhs, *rhs])))
            }
            (Some(Err(_)), _) => {
                partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![*lhs])))
            }
            (_, Some(Err(_))) => {
                partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![*rhs])))
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
        ast,
        data::ScopedType,
        error::{ExpectedShape, SemanticError},
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use lang::{
        types::{RefKind, ShallowType, Type},
        NodeId,
    };

    #[derive(Debug, PartialEq)]
    struct MockNode;

    #[test]
    fn skip_infer() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
            )],
            vec![],
        );
        let assert_skip =
            |op, lhs, rhs| assert_eq!(super::infer(&strong, op, lhs, rhs), partial::Action::Skip);

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
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
            )],
            vec![],
        );
        let assert_integer = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, op, lhs, rhs),
                partial::Action::Infer(&Ok(ScopedType::Type(Type::Integer)))
            )
        };

        assert_integer(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(0));
        assert_integer(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(0));
        assert_integer(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(0));
    }

    #[test]
    fn float_result() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Float))),
                ),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
            ],
            vec![],
        );
        let assert_float = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, op, lhs, rhs),
                partial::Action::Infer(&Ok(ScopedType::Type(Type::Float)))
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

    #[test]
    fn unexpected_shape() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(1),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Boolean))),
                ),
            ],
            vec![],
        );
        let assert_unexpected_shape = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, op, lhs, rhs),
                partial::Action::Infer(&Err(SemanticError::UnexpectedShape(
                    (ShallowType(Type::Boolean), NodeId(1)),
                    ExpectedShape::Union(vec![Type::Integer, Type::Float])
                )))
            )
        };

        assert_unexpected_shape(&ast::BinaryOperator::Add, &NodeId(0), &NodeId(1));
        assert_unexpected_shape(&ast::BinaryOperator::Add, &NodeId(1), &NodeId(0));

        assert_unexpected_shape(&ast::BinaryOperator::Subtract, &NodeId(0), &NodeId(1));
        assert_unexpected_shape(&ast::BinaryOperator::Subtract, &NodeId(1), &NodeId(0));

        assert_unexpected_shape(&ast::BinaryOperator::Multiply, &NodeId(0), &NodeId(1));
        assert_unexpected_shape(&ast::BinaryOperator::Multiply, &NodeId(1), &NodeId(0));
    }

    #[test]
    fn not_inferrable() {
        let strong = strong_result_from(
            vec![],
            vec![
                (
                    NodeId(0),
                    (RefKind::Value, Ok(ScopedType::Type(Type::Integer))),
                ),
                (
                    NodeId(1),
                    (RefKind::Value, Err(SemanticError::NotInferrable(vec![]))),
                ),
            ],
            vec![],
        );
        let assert_not_inferrable = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, op, lhs, rhs),
                partial::Action::Infer(&Err(SemanticError::NotInferrable(vec![NodeId(1)])))
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
