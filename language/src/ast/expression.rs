use super::{
    ksx::KSXNode,
    operator::{BinaryOperator, UnaryOperator},
    statement::StatementNode,
};
use crate::common::{node::Node, range::Range};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Primitive {
    Nil,
    Boolean(bool),
    Integer(i64),
    Float(f64, i32),
    String(String),
}

#[derive(Clone, Debug, PartialEq)]
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
        fe: &mut impl FnMut(&E) -> E2,
        fs: &mut impl FnMut(&S) -> S2,
        fk: &mut impl FnMut(&K) -> K2,
    ) -> Expression<E2, S2, K2> {
        match self {
            Self::Primitive(x) => Expression::Primitive(x.clone()),

            Self::Identifier(x) => Expression::Identifier(x.clone()),

            Self::Group(x) => Expression::Group(Box::new(fe(x))),

            Self::Closure(xs) => Expression::Closure(xs.iter().map(fs).collect()),

            Self::UnaryOperation(op, x) => Expression::UnaryOperation(op.clone(), Box::new(fe(x))),

            Self::BinaryOperation(op, lhs, rhs) => {
                Expression::BinaryOperation(op.clone(), Box::new(fe(lhs)), Box::new(fe(rhs)))
            }

            Self::DotAccess(lhs, rhs) => Expression::DotAccess(Box::new(fe(lhs)), rhs.clone()),

            Self::FunctionCall(lhs, arguments) => {
                Expression::FunctionCall(Box::new(fe(lhs)), arguments.iter().map(fe).collect())
            }

            Self::Style(xs) => Expression::Style(
                xs.iter()
                    .map(|(key, value)| (key.clone(), fe(value)))
                    .collect(),
            ),

            Self::KSX(x) => Expression::KSX(Box::new(fk(x))),
        }
    }
}

pub type ExpressionNodeValue<C> = Expression<ExpressionNode<C>, StatementNode<C>, KSXNode<C>>;

#[derive(Debug, PartialEq)]
pub struct ExpressionNode<C>(pub Node<ExpressionNodeValue<C>, C>);

impl<C> ExpressionNode<C> {
    pub fn node(&self) -> &Node<ExpressionNodeValue<C>, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&ExpressionNodeValue<C>, &C) -> (ExpressionNodeValue<C2>, C2),
    ) -> ExpressionNode<C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        ExpressionNode(Node(value, node.range().clone(), ctx))
    }
}

impl ExpressionNode<()> {
    pub fn raw(x: ExpressionNodeValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }
}
