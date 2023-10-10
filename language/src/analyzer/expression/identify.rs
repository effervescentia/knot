use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::{Expression, ExpressionNodeValue},
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<ExpressionNodeValue<T, NodeContext>> for ExpressionNodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> ExpressionNodeValue<T, NodeContext> {
        match self {
            Self::Closure(xs) => {
                let child_ctx = ctx.child();

                Expression::Closure(xs.iter().map(|x| x.register(&child_ctx)).collect())
            }

            _ => self.map(
                &mut |x| x.register(ctx),
                &mut |x| x.register(ctx),
                &mut |x| x.register(ctx),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{
            BinaryOperator, Expression, ExpressionNode, KSXNode, Primitive, Statement,
            StatementNode, UnaryOperator, KSX,
        },
        parser::CharStream,
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::<
                ExpressionNode<CharStream<'static>, ()>,
                StatementNode<CharStream<'static>, ()>,
                KSXNode<CharStream<'static>, ()>,
            >::Primitive(Primitive::Nil)
            .identify(scope),
            Expression::Primitive(Primitive::Nil)
        );
    }

    #[test]
    fn identifier() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::<
                ExpressionNode<CharStream<'static>, ()>,
                StatementNode<CharStream<'static>, ()>,
                KSXNode<CharStream<'static>, ()>,
            >::Identifier(String::from("foo"))
            .identify(scope),
            Expression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn group() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::Group(Box::new(f::n::x(Expression::Primitive(Primitive::Nil))))
                .identify(scope),
            Expression::Group(Box::new(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            )))
        );
    }

    #[test]
    fn closure() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::Closure(vec![
                f::n::s(Statement::Variable(
                    String::from("foo"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                )),
                f::n::s(Statement::Expression(f::n::x(Expression::Primitive(
                    Primitive::Nil
                )))),
            ])
            .identify(scope),
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
        );
    }

    #[test]
    fn unary_operation() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
            )
            .identify(scope),
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
            )
        );
    }

    #[test]
    fn binary_operation() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
            )
            .identify(scope),
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
        );
    }

    #[test]
    fn dot_access() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::DotAccess(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                String::from("foo"),
            )
            .identify(scope),
            Expression::DotAccess(
                Box::new(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                String::from("foo"),
            )
        );
    }

    #[test]
    fn function_call() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::FunctionCall(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                vec![
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ],
            )
            .identify(scope),
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
        );
    }

    #[test]
    fn style() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::Style(vec![
                (
                    String::from("foo"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ),
                (
                    String::from("bar"),
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                ),
            ])
            .identify(scope),
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
        );
    }

    #[test]
    fn ksx() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Expression::KSX(Box::new(f::n::kx(KSX::Text(String::from("foo"))))).identify(scope),
            Expression::KSX(Box::new(f::n::kxc(
                KSX::Text(String::from("foo")),
                NodeContext::new(0, vec![0])
            )))
        );
    }
}
