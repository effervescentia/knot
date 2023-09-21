use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::statement::{Statement, StatementNode},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToStrong<StatementNode<T, Strong>> for StatementNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self) -> StatementNode<T, Strong> {
        let node = self.node();
        StatementNode(Node(
            match node.0 {
                Statement::Effect(x) => Statement::Effect(x.to_strong()),

                Statement::Variable(name, x) => Statement::Variable(name, x.to_strong()),
            },
            node.1.clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
