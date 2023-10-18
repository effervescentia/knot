use super::ExpressionNode;
use crate::Node;
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

pub type StatementNodeValue<R, C> = Statement<ExpressionNode<R, C>>;

#[derive(Debug, PartialEq)]
pub struct StatementNode<R, C>(pub Node<StatementNodeValue<R, C>, R, C>);

impl<R, C> StatementNode<R, C>
where
    R: Clone,
{
    pub fn node(&self) -> &Node<StatementNodeValue<R, C>, R, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&StatementNodeValue<R, C>, &C) -> (StatementNodeValue<R, C2>, C2),
    ) -> StatementNode<R, C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        StatementNode(Node(value, node.range().clone(), ctx))
    }
}

impl<R> StatementNode<R, ()> {
    pub fn raw(x: StatementNodeValue<R, ()>, range: R) -> Self {
        Self(Node::raw(x, range))
    }
}
