mod fragment;
mod identity;
mod strong;
mod weak;

use super::{
    context::NodeContext,
    infer::weak::Weak,
    register::{Identify, Register},
    RefKind, ScopeContext, Type,
};
use crate::{
    ast::statement::{self, StatementNode},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for StatementNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = StatementNode<T, NodeContext>;
    type Value<C> = statement::NodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let node = &self.0;
        let value = node.value().identify(ctx);
        let id = ctx.add_fragment(&value);

        StatementNode(Node(value, node.range().clone(), id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        ast::{
            expression::{Expression, Primitive},
            statement::Statement,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;

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
            BTreeMap::from_iter(vec![
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
