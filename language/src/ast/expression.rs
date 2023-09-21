use super::{
    ksx::KSXNode,
    operator::{BinaryOperator, UnaryOperator},
    statement::StatementNode,
};
use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Primitive {
    Nil,
    Boolean(bool),
    Integer(i64),
    Float(f64, i32),
    String(String),
}

#[derive(Debug, PartialEq)]
pub enum Expression<E, S, K> {
    Primitive(Primitive),
    Identifier(String),
    Group(Box<E>),
    Closure(Vec<S>),
    UnaryOperation(UnaryOperator, Box<E>),
    BinaryOperation(BinaryOperator, Box<E>, Box<E>),
    DotAccess(Box<E>, String),
    FunctionCall(Box<E>, Vec<E>),
    Style(Vec<(String, E)>),
    KSX(Box<K>),
}

pub type NodeValue<T, C> = Expression<ExpressionNode<T, C>, StatementNode<T, C>, KSXNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct ExpressionNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> ExpressionNode<T, C>
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
    ) -> ExpressionNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(&node.0, &node.2);

        ExpressionNode(Node(value, node.1.clone(), ctx))
    }
}

impl<T> ExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}
