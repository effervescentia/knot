use super::partial;
use crate::{
    ast,
    data::ScopedType,
    error::{ExpectedType, SemanticError},
    strong,
};
use kore::invariant;
use lang::{
    types::{RefKind, Type, TypeShape},
    NodeId,
};

pub fn infer<'a, Node>(
    strong: &strong::Result,
    node: Node,
    op: &ast::BinaryOperator,
    lhs: &NodeId,
    rhs: &NodeId,
) -> partial::Action<'a, Node> {
    match op {
        ast::BinaryOperator::Add
        | ast::BinaryOperator::Subtract
        | ast::BinaryOperator::Multiply => match (
            strong.get_type_shape(lhs, &RefKind::Value),
            strong.get_type_shape(&rhs, &RefKind::Value),
        ) {
            (Some(Ok(TypeShape(Type::Integer))), Some(Ok(TypeShape(Type::Integer)))) => {
                partial::Action::Infer(node, &Ok(ScopedType::Type(Type::Integer)))
            }

            (
                Some(Ok(TypeShape(Type::Integer | Type::Float))),
                Some(Ok(TypeShape(Type::Integer | Type::Float))),
            ) => partial::Action::Infer(node, &Ok(ScopedType::Type(Type::Float))),

            (Some(Ok(TypeShape(Type::Integer | Type::Float))), Some(Ok(rhs_type))) => {
                partial::Action::Infer(
                    node,
                    &Err(SemanticError::UnexpectedShape(
                        (rhs_type.clone(), *rhs),
                        ExpectedType::Union(vec![Type::Integer, Type::Float]),
                    )),
                )
            }

            (Some(Ok(lhs_type)), Some(_)) => partial::Action::Infer(
                node,
                &Err(SemanticError::UnexpectedShape(
                    (lhs_type.clone(), *lhs),
                    ExpectedType::Union(vec![Type::Integer, Type::Float]),
                )),
            ),

            (Some(Err(_)), Some(Err(_))) => {
                partial::Action::Infer(node, &Err(SemanticError::NotInferrable(vec![*lhs, *rhs])))
            }
            (Some(Err(_)), _) => {
                partial::Action::Infer(node, &Err(SemanticError::NotInferrable(vec![*lhs])))
            }
            (_, Some(Err(_))) => {
                partial::Action::Infer(node, &Err(SemanticError::NotInferrable(vec![*rhs])))
            }

            (_, None) | (None, _) => partial::Action::Skip(node),
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
        data::ScopedType,
        error::{ExpectedType, SemanticError},
        infer::strong::partial,
        test::fixture::strong_result_from,
    };
    use lang::{types, NodeId};

    #[derive(Debug, PartialEq)]
    struct MockNode;

    #[test]
    fn skip_infer() {
        let strong = strong_result_from(
            vec![],
            vec![(
                NodeId(0),
                (
                    types::RefKind::Value,
                    Ok(ScopedType::Type(types::Type::Integer)),
                ),
            )],
            vec![],
        );
        let assert_skip = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, MockNode, op, lhs, rhs),
                partial::Action::Skip(MockNode)
            )
        };

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
                (
                    types::RefKind::Value,
                    Ok(ScopedType::Type(types::Type::Integer)),
                ),
            )],
            vec![],
        );
        let assert_integer = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, MockNode, op, lhs, rhs),
                partial::Action::Infer(MockNode, &Ok(ScopedType::Type(types::Type::Integer)))
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
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Float)),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Integer)),
                    ),
                ),
            ],
            vec![],
        );
        let assert_float = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, MockNode, op, lhs, rhs),
                partial::Action::Infer(MockNode, &Ok(ScopedType::Type(types::Type::Float)))
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
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Integer)),
                    ),
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Boolean)),
                    ),
                ),
            ],
            vec![],
        );
        let assert_unexpected_shape = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, MockNode, op, lhs, rhs),
                partial::Action::Infer(
                    MockNode,
                    &Err(SemanticError::UnexpectedShape(
                        (types::TypeShape(types::Type::Boolean), NodeId(1)),
                        ExpectedType::Union(vec![types::Type::Integer, types::Type::Float])
                    ))
                )
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
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Integer)),
                    ),
                ),
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
        let assert_not_inferrable = |op, lhs, rhs| {
            assert_eq!(
                super::infer(&strong, MockNode, op, lhs, rhs),
                partial::Action::Infer(
                    MockNode,
                    &Err(SemanticError::NotInferrable(vec![NodeId(1)]))
                )
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
