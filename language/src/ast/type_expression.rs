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

impl<T> TypeExpression<T> {
    pub fn map<T2>(&self, ft: &impl Fn(&T) -> T2) -> TypeExpression<T2> {
        match self {
            Self::Nil => TypeExpression::Nil,
            Self::Boolean => TypeExpression::Boolean,
            Self::Integer => TypeExpression::Integer,
            Self::Float => TypeExpression::Float,
            Self::String => TypeExpression::String,
            Self::Style => TypeExpression::Style,
            Self::Element => TypeExpression::Element,
            Self::Identifier(x) => TypeExpression::Identifier(x.clone()),
            Self::Group(x) => TypeExpression::Group(Box::new(ft(x))),
            Self::DotAccess(lhs, rhs) => TypeExpression::DotAccess(Box::new(ft(lhs)), rhs.clone()),
            Self::Function(parameters, x) => TypeExpression::Function(
                parameters.iter().map(|x| ft(x)).collect(),
                Box::new(ft(x)),
            ),
        }
    }
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

    pub fn map<R>(
        &self,
        f: impl Fn(&NodeValue<T, C>, &C) -> (NodeValue<T, R>, R),
    ) -> TypeExpressionNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        TypeExpressionNode(Node(value, node.range().clone(), ctx))
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
