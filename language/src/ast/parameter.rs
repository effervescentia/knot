use crate::{
    ast::{expression::ExpressionNode, type_expression::TypeExpressionNode},
    common::{node::Node, position::Decrement, range::Range},
};
use combine::Stream;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
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

    pub fn map<E2, T2>(&self, fe: &impl Fn(&E) -> E2, ft: &impl Fn(&T) -> T2) -> Parameter<E2, T2> {
        Parameter {
            name: self.name.clone(),
            value_type: self.value_type.as_ref().map(ft),
            default_value: self.default_value.as_ref().map(fe),
        }
    }
}

pub type NodeValue<T, C> = Parameter<ExpressionNode<T, C>, TypeExpressionNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct ParameterNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> ParameterNode<T, C>
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
    ) -> ParameterNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        ParameterNode(Node(value, node.range().clone(), ctx))
    }
}

impl<T> ParameterNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node(x, range, ()))
    }
}
