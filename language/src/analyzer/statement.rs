use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
use crate::parser::{
    expression::{
        statement::{Statement, StatementNode},
        ExpressionNode,
    },
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Analyze for StatementNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Ref = Statement<usize>;
    type Node = StatementNode<T, NodeContext>;
    type Value<C> = Statement<ExpressionNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::Statement(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        StatementNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
        match value {
            Statement::Effect(x) => Statement::Effect(x.register(ctx)),

            Statement::Variable(name, x) => Statement::Variable(name, x.register(ctx)),
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Self::Ref {
        match value {
            Statement::Effect(x) => Statement::Effect(*x.node().id()),

            Statement::Variable(name, x) => Statement::Variable(name.clone(), *x.node().id()),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::expression::{primitive::Primitive, statement::Statement, Expression},
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn effect() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::sc(
                Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), ())),
                ()
            )
            .register(scope),
            f::sc(
                Statement::Effect(f::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                NodeContext::new(1, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (1, (vec![0], Fragment::Statement(Statement::Effect(0))))
            ])
        );
    }

    #[test]
    fn variable() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::sc(
                Statement::Variable(
                    String::from("foo"),
                    f::xc(Expression::Primitive(Primitive::Nil), ())
                ),
                ()
            )
            .register(scope),
            f::sc(
                Statement::Variable(
                    String::from("foo"),
                    f::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )
                ),
                NodeContext::new(1, vec![0])
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
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
