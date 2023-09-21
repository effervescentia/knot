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
    ast::type_expression::{self, TypeExpressionNode},
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
    type Value<C> = type_expression::NodeValue<T, C>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node {
        let node = self.0;
        let value = node.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        TypeExpressionNode(Node(value, node.1, id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        ast::type_expression::TypeExpression,
        test::fixture as f,
    };
    use std::collections::BTreeMap;

    #[test]
    fn register() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

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
            BTreeMap::from_iter(vec![
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
