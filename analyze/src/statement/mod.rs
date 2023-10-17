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
use lang::{
    ast::{StatementNode, StatementNodeValue},
    Node,
};

impl Register for StatementNode<()> {
    type Node = StatementNode<NodeContext>;
    type Value<C> = StatementNodeValue<C>;

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
        context::{FragmentMap, NodeContext},
        fragment::Fragment,
        register::Register,
        test::fixture::{file_ctx, scope_ctx},
    };
    use lang::{
        ast::{Expression, Primitive, Statement},
        test::fixture as f,
    };

    #[test]
    fn register() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

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
