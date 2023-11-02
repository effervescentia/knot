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
use lang::ast::{AstNode, StatementNode, StatementNodeValue};

impl<R> Register for StatementNode<R, ()>
where
    R: Copy,
{
    type Node = StatementNode<R, NodeContext>;
    type Value<C> = StatementNodeValue<R, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let node = self.node();
        let value = node.value().identify(ctx);
        let id = ctx.add_fragment(&value);

        StatementNode::new(value, *node.range(), id)
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::{FragmentMap, NodeContext},
        fragment::Fragment,
        register::Register,
        test::fixture as f,
    };
    use lang::ast::{Expression, Primitive, Statement};

    #[test]
    fn register() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
