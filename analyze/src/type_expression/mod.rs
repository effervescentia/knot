mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::NodeContext,
    register::{Identify, Register},
    ScopeContext,
};
use lang::{
    ast::{TypeExpressionNode, TypeExpressionNodeValue},
    Node,
};

impl<R> Register for TypeExpressionNode<R, ()>
where
    R: Copy,
{
    type Node = TypeExpressionNode<R, NodeContext>;
    type Value<C> = TypeExpressionNodeValue<R, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let value = self.node().value().identify(ctx);
        let id = ctx.add_fragment(&value);

        TypeExpressionNode(Node(value, *self.node().range(), id))
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
    use lang::ast::TypeExpression;
    use parse::test::fixture as f;

    #[test]
    fn register() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::n::tx(TypeExpression::Function(
                vec![f::n::tx(TypeExpression::Nil), f::n::tx(TypeExpression::Nil),],
                Box::new(f::n::tx(TypeExpression::Nil)),
            ))
            .register(scope),
            f::n::txc(
                TypeExpression::Function(
                    vec![
                        f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0])),
                        f::n::txc(TypeExpression::Nil, NodeContext::new(1, vec![0])),
                    ],
                    Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(2, vec![0]))),
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
                (0, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (1, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (2, (vec![0], Fragment::TypeExpression(TypeExpression::Nil))),
                (
                    3,
                    (
                        vec![0],
                        Fragment::TypeExpression(TypeExpression::Function(vec![0, 1], Box::new(2)))
                    )
                )
            ])
        );
    }
}
