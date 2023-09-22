use crate::{
    ast::expression::ExpressionNode,
    common::{node::Node, position::Decrement, range::Range},
};
use combine::Stream;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Statement<E> {
    Expression(E),
    Variable(String, E),
}

impl<E> Statement<E> {
    pub fn map<E2>(&self, fe: &impl Fn(&E) -> E2) -> Statement<E2> {
        match self {
            Self::Expression(x) => Statement::Expression(fe(x)),

            Self::Variable(name, x) => Statement::Variable(name.clone(), fe(x)),
        }
    }
}

pub type NodeValue<T, C> = Statement<ExpressionNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct StatementNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> StatementNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<NodeValue<T, C>, T, C> {
        &self.0
    }

    pub fn map<R>(
        &self,
        f: impl Fn(&NodeValue<T, C>, &C) -> (NodeValue<T, R>, R),
    ) -> StatementNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        StatementNode(Node(value, node.range().clone(), ctx))
    }
}

impl<T> StatementNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}
