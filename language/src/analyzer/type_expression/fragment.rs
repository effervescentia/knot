use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    ast::type_expression,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for type_expression::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::TypeExpression(self.map(&|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        ast::type_expression::{TypeExpression, TypeExpressionNode},
        parser::CharStream,
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
