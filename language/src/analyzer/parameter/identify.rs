use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::ParameterNodeValue,
};

impl Identify<ParameterNodeValue<NodeContext>> for ParameterNodeValue<()> {
    fn identify(&self, ctx: &ScopeContext) -> ParameterNodeValue<NodeContext> {
        self.map(&mut |x| x.register(ctx), &mut |x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{Expression, Parameter, Primitive, TypeExpression},
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
