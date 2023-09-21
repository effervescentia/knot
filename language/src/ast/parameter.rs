use crate::{
    ast::{expression::ExpressionNode, type_expression::TypeExpressionNode},
    common::{node::Node, position::Decrement, range::Range},
};
use combine::Stream;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Parameter<E, T> {
    pub name: String,
    pub value_type: Option<T>,
    pub default_value: Option<E>,
}

impl<E, T> Parameter<E, T> {
    pub fn new(name: String, value_type: Option<T>, default_value: Option<E>) -> Self {
        Self {
            name,
            value_type,
            default_value,
        }
    }
}

pub type NodeValue<T, C> = Parameter<ExpressionNode<T, C>, TypeExpressionNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct ParameterNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T> ParameterNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node(x, range, ()))
    }
}
