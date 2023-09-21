use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::parameter,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<parameter::NodeValue<T, NodeContext>> for parameter::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> parameter::NodeValue<T, NodeContext> {
        self.map(&mut |x| x.register(ctx), &mut |x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{
            expression::{Expression, Primitive},
            parameter::Parameter,
            type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
