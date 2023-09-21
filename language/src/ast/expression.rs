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

impl<E, S, K> Expression<E, S, K> {
    pub fn map<E2, S2, K2>(
        &self,
        fe: &impl Fn(&E) -> E2,
        fs: &impl Fn(&S) -> S2,
        fk: &impl Fn(&K) -> K2,
    ) -> Expression<E2, S2, K2> {
        match self {
            Self::Primitive(x) => Expression::Primitive(x.clone()),

            Self::Identifier(x) => Expression::Identifier(x.clone()),

            Self::Group(x) => Expression::Group(Box::new(fe(x))),

            Self::Closure(xs) => Expression::Closure(xs.iter().map(fs).collect::<Vec<_>>()),

            Self::UnaryOperation(op, x) => Expression::UnaryOperation(op.clone(), Box::new(fe(x))),

            Self::BinaryOperation(op, lhs, rhs) => {
                Expression::BinaryOperation(op.clone(), Box::new(fe(lhs)), Box::new(fe(rhs)))
            }

            Self::DotAccess(lhs, rhs) => Expression::DotAccess(Box::new(fe(lhs)), rhs.clone()),

            Self::FunctionCall(x, xs) => {
                Expression::FunctionCall(Box::new(fe(x)), xs.iter().map(fe).collect::<Vec<_>>())
            }

            Self::Style(xs) => Expression::Style(
                xs.iter()
                    .map(|(key, value)| (key.clone(), fe(value)))
                    .collect::<Vec<_>>(),
            ),

            Self::KSX(x) => Expression::KSX(Box::new(fk(x))),
        }
    }
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
        let (value, ctx) = f(node.value(), node.context());

        ExpressionNode(Node(value, node.range().clone(), ctx))
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
