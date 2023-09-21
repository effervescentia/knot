use super::{
    expression::ExpressionNode, module::ModuleNode, parameter::ParameterNode, storage::Storage,
    type_expression::TypeExpressionNode,
};
use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Declaration<E, P, M, T> {
    TypeAlias {
        name: Storage,
        value: T,
    },
    Constant {
        name: Storage,
        value_type: Option<T>,
        value: E,
    },
    Enumerated {
        name: Storage,
        variants: Vec<(String, Vec<T>)>,
    },
    Function {
        name: Storage,
        parameters: Vec<P>,
        body_type: Option<T>,
        body: E,
    },
    View {
        name: Storage,
        parameters: Vec<P>,
        body: E,
    },
    Module {
        name: Storage,
        value: M,
    },
}

pub type NodeValue<T, C> = Declaration<
    ExpressionNode<T, C>,
    ParameterNode<T, C>,
    ModuleNode<T, C>,
    TypeExpressionNode<T, C>,
>;

#[derive(Debug, PartialEq)]
pub struct DeclarationNode<T, C>(pub Node<NodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> DeclarationNode<T, C>
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
    ) -> DeclarationNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(&node.0, &node.2);

        DeclarationNode(Node(value, node.1.clone(), ctx))
    }
}

impl<T> DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: NodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}
