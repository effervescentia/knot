use crate::{
    ast::{expression::ExpressionNode, type_expression::TypeExpressionNode},
    common::{node::Node, range::Range},
};
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

pub type ParameterNodeValue<C> = Parameter<ExpressionNode<C>, TypeExpressionNode<C>>;

#[derive(Debug, PartialEq)]
pub struct ParameterNode<C>(pub Node<ParameterNodeValue<C>, C>);

impl<C> ParameterNode<C> {
    pub fn node(&self) -> &Node<ParameterNodeValue<C>, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&ParameterNodeValue<C>, &C) -> (ParameterNodeValue<C2>, C2),
    ) -> ParameterNode<C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        ParameterNode(Node(value, node.range().clone(), ctx))
    }
}

impl ParameterNode<()> {
    pub fn raw(x: ParameterNodeValue<()>, range: Range) -> Self {
        Self(Node(x, range, ()))
    }
}
