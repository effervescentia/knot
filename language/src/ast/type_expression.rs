use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Identifier(String),
    Group(Box<T>),
    DotAccess(Box<T>, String),
    Function(Vec<T>, Box<T>),
    // View(Vec<(String, TypeExpression)>),
}

pub type NodeValue<T, C> = TypeExpression<TypeExpressionNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct TypeExpressionNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> TypeExpressionNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<NodeValue<T, C>, T, C> {
        &self.0
    }
}

impl<T> TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}
