use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    ast::expression,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for expression::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Expression(
            self.map(&mut |x| *x.node().id(), &mut |x| *x.node().id(), &mut |x| {
                *x.node().id()
            }),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
        ast::{
            expression::{Expression, ExpressionNode, Primitive},
            ksx::{KSXNode, KSX},
            operator::{BinaryOperator, UnaryOperator},
            statement::{Statement, StatementNode},
        },
        parser::CharStream,
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        assert_eq!(
            Expression::<
                ExpressionNode<CharStream<'static>, NodeContext>,
                StatementNode<CharStream<'static>, NodeContext>,
                KSXNode<CharStream<'static>, NodeContext>,
            >::Primitive(Primitive::Nil)
            .to_fragment(),
            Fragment::Expression(Expression::Primitive(Primitive::Nil))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            Expression::<
                ExpressionNode<CharStream<'static>, NodeContext>,
                StatementNode<CharStream<'static>, NodeContext>,
                KSXNode<CharStream<'static>, NodeContext>,
            >::Identifier(String::from("foo"))
            .to_fragment(),
            Fragment::Expression(Expression::Identifier(String::from("foo")))
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            Expression::Group(Box::new(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            )))
            .to_fragment(),
            Fragment::Expression(Expression::Group(Box::new(0)))
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            Expression::Closure(vec![
                f::n::sc(
                    Statement::Variable(
                        String::from("foo"),
                        f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(0, vec![0, 1])
                        ),
                    ),
                    NodeContext::new(1, vec![0, 1])
                ),
                f::n::sc(
                    Statement::Expression(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(2, vec![0, 1])
                    )),
                    NodeContext::new(3, vec![0, 1])
                )
            ])
            .to_fragment(),
            Fragment::Expression(Expression::Closure(vec![1, 3]))
        );
    }

    #[test]
    fn unary_operation() {
        assert_eq!(
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
            )
            .to_fragment(),
            Fragment::Expression(Expression::UnaryOperation(UnaryOperator::Not, Box::new(0)))
        );
    }

    #[test]
    fn binary_operation() {
        assert_eq!(
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                )),
            )
            .to_fragment(),
            Fragment::Expression(Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(0),
                Box::new(1),
            ))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            Expression::DotAccess(
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                String::from("foo"),
            )
            .to_fragment(),
            Fragment::Expression(Expression::DotAccess(Box::new(0), String::from("foo")))
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            Expression::FunctionCall(
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                vec![
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(1, vec![0])
                    ),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(2, vec![0])
                    ),
                ],
            )
            .to_fragment(),
            Fragment::Expression(Expression::FunctionCall(Box::new(0), vec![1, 2]))
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            Expression::Style(vec![
                (
                    String::from("foo"),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    ),
                ),
                (
                    String::from("bar"),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(1, vec![0])
                    ),
                ),
            ])
            .to_fragment(),
            Fragment::Expression(Expression::Style(vec![
                (String::from("foo"), 0),
                (String::from("bar"), 1),
            ]))
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            Expression::KSX(Box::new(f::n::kxc(
                KSX::Text(String::from("foo")),
                NodeContext::new(0, vec![0])
            )))
            .to_fragment(),
            Fragment::Expression(Expression::KSX(Box::new(0)))
        );
    }
}
