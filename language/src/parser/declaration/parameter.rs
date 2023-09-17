use crate::parser::{
    expression::{self, ExpressionNode},
    matcher as m,
    node::Node,
    position::Decrement,
    range::Range,
    types::{type_expression::TypeExpressionNode, typedef},
};
use combine::{optional, Parser, Stream};
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

pub fn parameter<T>() -> impl Parser<T, Output = ParameterNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        m::standard_identifier(),
        typedef::typedef(),
        optional(m::symbol('=').with(expression::expression())),
    )
        .map(|((name, start), value_type, default_value)| {
            let mut range = start;

            if let Some(x) = &value_type {
                range = &range + x.node().range();
            }

            if let Some(x) = &default_value {
                range = &range + x.node().range();
            }

            ParameterNode::raw(Parameter::new(name, value_type, default_value), range)
        })
}
