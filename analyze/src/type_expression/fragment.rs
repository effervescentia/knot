use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::{AstNode, TypeExpressionNodeValue};

impl<R> ToFragment for TypeExpressionNodeValue<R, NodeContext>
where
    R: Copy,
{
    fn to_fragment(&self) -> Fragment {
        Fragment::TypeExpression(self.map(&|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext, fragment::Fragment, register::ToFragment, test::fixture as f,
    };
    use lang::ast::{TypeExpression, TypeExpressionNode};
    use parse::Range;

    #[test]
    fn primitive() {
        assert_eq!(
            TypeExpression::<TypeExpressionNode<Range, NodeContext>>::Nil.to_fragment(),
            Fragment::TypeExpression(TypeExpression::Nil)
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            TypeExpression::<TypeExpressionNode<Range, NodeContext>>::Identifier(String::from(
                "foo"
            ))
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
