use crate::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::ParameterNodeValue;

impl Identify<ParameterNodeValue<NodeContext>> for ParameterNodeValue<()> {
    fn identify(&self, ctx: &ScopeContext) -> ParameterNodeValue<NodeContext> {
        self.map(&mut |x| x.register(ctx), &mut |x| x.register(ctx))
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
        ast::{Expression, Parameter, Primitive, TypeExpression},
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            Parameter::new(
                String::from("foo"),
                Some(f::n::tx(TypeExpression::Nil)),
                Some(f::n::x(Expression::Primitive(Primitive::Nil)))
            )
            .identify(scope),
            Parameter::new(
                String::from("foo"),
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                Some(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                ))
            )
        );
    }
}
