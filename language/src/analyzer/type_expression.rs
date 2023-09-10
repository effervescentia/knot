use super::Context;
use crate::parser::{
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_type_expression<T>(
    x: TypeExpressionNode<T, ()>,
    ctx: &mut Context,
) -> TypeExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    TypeExpressionNode(
        x.0.map(|x| match x {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

            TypeExpression::Group(x) => {
                TypeExpression::Group(Box::new(analyze_type_expression(*x, ctx)))
            }

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new(analyze_type_expression(*lhs, ctx)), rhs)
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params
                    .into_iter()
                    .map(|x| analyze_type_expression(x, ctx))
                    .collect::<Vec<_>>(),
                Box::new(analyze_type_expression(*body, ctx)),
            ),
        })
        .with_context(ctx.generate_id()),
    )
}

#[cfg(test)]
mod tests {
    use super::analyze_type_expression;
    use crate::{
        analyzer::Context,
        parser::{
            node::Node,
            range::Range,
            types::type_expression::{TypeExpression, TypeExpressionNode},
            CharStream,
        },
    };

    const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

    #[test]
    fn primitive() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ())),
            ctx,
        );

        assert_eq!(
            result,
            TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))
        )
    }

    #[test]
    fn identifier() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            TypeExpressionNode(Node(
                TypeExpression::Identifier(String::from("foo")),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            TypeExpressionNode(Node(
                TypeExpression::Identifier(String::from("foo")),
                RANGE,
                0
            ))
        )
    }

    #[test]
    fn group() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            TypeExpressionNode(Node(
                TypeExpression::Group(Box::new(TypeExpressionNode(Node(
                    TypeExpression::Nil,
                    RANGE,
                    (),
                )))),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            TypeExpressionNode(Node(
                TypeExpression::Group(Box::new(TypeExpressionNode(Node(
                    TypeExpression::Nil,
                    RANGE,
                    0,
                )))),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn dot_access() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            TypeExpressionNode(Node(
                TypeExpression::DotAccess(
                    Box::new(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                    String::from("foo"),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            TypeExpressionNode(Node(
                TypeExpression::DotAccess(
                    Box::new(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0))),
                    String::from("foo"),
                ),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        let result = analyze_type_expression(
            TypeExpressionNode(Node(
                TypeExpression::Function(
                    vec![
                        TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ())),
                        TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ())),
                    ],
                    Box::new(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, ()))),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            TypeExpressionNode(Node(
                TypeExpression::Function(
                    vec![
                        TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 0)),
                        TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 1)),
                    ],
                    Box::new(TypeExpressionNode(Node(TypeExpression::Nil, RANGE, 2))),
                ),
                RANGE,
                3,
            ))
        )
    }
}
