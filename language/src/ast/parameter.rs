use super::{ExpressionNode, TypeExpressionNode};
use crate::Node;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter<E, T> {
    pub name: String,
    pub value_type: Option<T>,
    pub default_value: Option<E>,
}

impl<E, T> Parameter<E, T> {
    pub const fn new(name: String, value_type: Option<T>, default_value: Option<E>) -> Self {
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

pub type ParameterNodeValue<R, C> = Parameter<ExpressionNode<R, C>, TypeExpressionNode<R, C>>;

#[derive(Debug, PartialEq)]
pub struct ParameterNode<R, C>(pub Node<ParameterNodeValue<R, C>, R, C>);

impl<R, C> ParameterNode<R, C>
where
    R: Copy,
{
    pub const fn node(&self) -> &Node<ParameterNodeValue<R, C>, R, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&ParameterNodeValue<R, C>, &C) -> (ParameterNodeValue<R, C2>, C2),
    ) -> ParameterNode<R, C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        ParameterNode(Node(value, *node.range(), ctx))
    }
}

impl<R> ParameterNode<R, ()> {
    pub const fn raw(x: ParameterNodeValue<R, ()>, range: R) -> Self {
        Self(Node(x, range, ()))
    }
}
