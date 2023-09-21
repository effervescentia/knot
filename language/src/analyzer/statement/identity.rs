use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::statement,
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Identify<statement::NodeValue<T, NodeContext>> for statement::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> statement::NodeValue<T, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
        },
        test::fixture as f,
    };

    #[test]
    fn effect() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            Statement::Effect(f::n::x(Expression::Primitive(Primitive::Nil))).identify(scope),
            Statement::Effect(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
        );
    }

    #[test]
    fn variable() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
