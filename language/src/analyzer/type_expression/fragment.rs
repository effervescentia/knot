use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        position::Decrement,
        types::type_expression::{self, TypeExpression},
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for type_expression::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::TypeExpression(match self {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x.clone()),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new(*(*x).0.id())),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new(*(*lhs).0.id()), rhs.clone())
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>(),
                Box::new(*body.0.id()),
            ),
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        parser::{
            types::type_expression::{TypeExpression, TypeExpressionNode},
            CharStream,
        },
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        assert_eq!(
            TypeExpression::<TypeExpressionNode<CharStream<'static>, NodeContext>>::Nil
                .to_fragment(),
            Fragment::TypeExpression(TypeExpression::Nil)
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            TypeExpression::<TypeExpressionNode<CharStream<'static>, NodeContext>>::Identifier(
                String::from("foo")
            )
            .to_fragment(),
            Fragment::TypeExpression(TypeExpression::Identifier(String::from("foo")))
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            TypeExpression::Group(Box::new(f::n::txc(
                TypeExpression::Nil,
                NodeContext::new(0, vec![0])
            )))
            .to_fragment(),
            Fragment::TypeExpression(TypeExpression::Group(Box::new(0)))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            TypeExpression::DotAccess(
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                String::from("foo"),
            )
            .to_fragment(),
            Fragment::TypeExpression(TypeExpression::DotAccess(Box::new(0), String::from("foo"),))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            TypeExpression::Function(
                vec![
                    f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0])),
                    f::n::txc(TypeExpression::Nil, NodeContext::new(1, vec![0])),
                ],
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(2, vec![0]))),
            )
            .to_fragment(),
            Fragment::TypeExpression(TypeExpression::Function(vec![0, 1], Box::new(2)))
        );
    }
}
