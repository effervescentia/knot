use crate::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::ParameterNodeValue;

impl<R> Identify<ParameterNodeValue<R, NodeContext>> for ParameterNodeValue<R, ()>
where
    R: Copy,
{
    fn identify(&self, ctx: &ScopeContext) -> ParameterNodeValue<R, NodeContext> {
        self.map(&|x| x.register(ctx), &|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{context::NodeContext, register::Identify, test::fixture as f};
    use kore::str;
    use lang::ast::{Expression, Parameter, Primitive, TypeExpression};

    #[test]
    fn parameter() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            Parameter::new(
                str!("foo"),
                Some(f::n::tx(TypeExpression::Nil)),
                Some(f::n::x(Expression::Primitive(Primitive::Nil)))
            )
            .identify(scope),
            Parameter::new(
                str!("foo"),
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                Some(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                ))
            )
        );
    }
}
