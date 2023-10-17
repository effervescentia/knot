use crate::{
    ast::expression::ExpressionNode,
    common::{node::Node, range::Range},
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
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

pub type StatementNodeValue<C> = Statement<ExpressionNode<C>>;

#[derive(Debug, PartialEq)]
pub struct StatementNode<C>(pub Node<StatementNodeValue<C>, C>);

impl<C> StatementNode<C> {
    pub fn node(&self) -> &Node<StatementNodeValue<C>, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&StatementNodeValue<C>, &C) -> (StatementNodeValue<C2>, C2),
    ) -> StatementNode<C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        StatementNode(Node(value, node.range().clone(), ctx))
    }
}

impl StatementNode<()> {
    pub fn raw(x: StatementNodeValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }
}
