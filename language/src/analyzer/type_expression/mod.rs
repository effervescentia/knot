mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::NodeContext,
    register::{Identify, Register},
    ScopeContext,
};
use crate::{
    ast::{TypeExpressionNode, TypeExpressionNodeValue},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = TypeExpressionNode<T, NodeContext>;
    type Value<C> = TypeExpressionNodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let value = self.node().value().identify(ctx);
        let id = ctx.add_fragment(&value);

        TypeExpressionNode(Node(value, self.node().range().clone(), id))
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
        ast::TypeExpression,
        test::fixture as f,
    };

    #[test]
    fn register() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
