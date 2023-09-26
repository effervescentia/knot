mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use crate::{
    ast::parameter::{self, ParameterNode},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for ParameterNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = ParameterNode<T, NodeContext>;
    type Value<C> = parameter::NodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let value = self.node().value().identify(ctx);
        let id = ctx.add_fragment(&value);

        ParameterNode(Node(value, self.node().range().clone(), id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            context::{FragmentMap, NodeContext},
            fragment::Fragment,
            register::Register,
        },
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
        },
        test::fixture as f,
    };

    #[test]
    fn register() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::s(Statement::Variable(
                String::from("foo"),
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .register(scope),
            f::n::sc(
                Statement::Variable(
                    String::from("foo"),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )
                ),
                NodeContext::new(1, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Statement(Statement::Variable(String::from("foo"), 0))
                    )
                )
            ])
        );
    }
}
