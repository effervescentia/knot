use crate::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::StatementNodeValue;

impl Identify<StatementNodeValue<NodeContext>> for StatementNodeValue<()> {
    fn identify(&self, ctx: &ScopeContext) -> StatementNodeValue<NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext,
        register::Identify,
        test::fixture::{file_ctx, scope_ctx},
    };
    use lang::{
        ast::{Expression, Primitive, Statement},
        test::fixture as f,
    };

    #[test]
    fn expression() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

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
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

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
