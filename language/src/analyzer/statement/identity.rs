use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::StatementNodeValue,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<StatementNodeValue<T, NodeContext>> for StatementNodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> StatementNodeValue<T, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{Expression, Primitive, Statement},
        test::fixture as f,
    };

    #[test]
    fn expression() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Statement::Expression(f::n::x(Expression::Primitive(Primitive::Nil))).identify(scope),
            Statement::Expression(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
        );
    }

    #[test]
    fn variable() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Statement::Variable(
                String::from("foo"),
                f::n::x(Expression::Primitive(Primitive::Nil))
            )
            .identify(scope),
            Statement::Variable(
                String::from("foo"),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )
            )
        );
    }
}
