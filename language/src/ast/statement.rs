use super::{ExpressionNode, TypedNode};
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

pub type StatementNode<R, C> = TypedNode<Statement<ExpressionNode<R, C>>, R, C>;
