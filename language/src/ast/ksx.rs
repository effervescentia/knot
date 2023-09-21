use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

use super::expression::ExpressionNode;

#[derive(Debug, PartialEq)]
pub enum KSX<E, K> {
    Fragment(Vec<K>),
    OpenElement(String, Vec<(String, Option<E>)>, Vec<K>, String),
    ClosedElement(String, Vec<(String, Option<E>)>),
    Inline(E),
    Text(String),
}

pub type NodeValue<T, C> = KSX<ExpressionNode<T, C>, KSXNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct KSXNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> KSXNode<T, C>
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
    ) -> KSXNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(&node.0, &node.2);

        KSXNode(Node(value, node.1.clone(), ctx))
    }
}

impl<T> KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }

    pub fn bind((x, range): (NodeValue<T, ()>, Range<T>)) -> Self {
        Self::raw(x, range)
    }
}
