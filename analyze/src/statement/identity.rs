use crate::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::StatementNodeValue;

impl<R> Identify<StatementNodeValue<R, NodeContext>> for StatementNodeValue<R, ()>
where
    R: Clone,
{
    fn identify(&self, ctx: &ScopeContext) -> StatementNodeValue<R, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{context::NodeContext, register::Identify, test::fixture as f};
    use lang::ast::{Expression, Primitive, Statement};

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
