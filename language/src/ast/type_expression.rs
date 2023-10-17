use crate::common::{node::Node, range::Range};
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
            Self::Function(parameters, x) => {
                TypeExpression::Function(parameters.iter().map(ft).collect(), Box::new(ft(x)))
            }
        }
    }
}

pub type TypeExpressionNodeValue<C> = TypeExpression<TypeExpressionNode<C>>;

#[derive(Debug, PartialEq)]
pub struct TypeExpressionNode<C>(pub Node<TypeExpressionNodeValue<C>, C>);

impl<C> TypeExpressionNode<C> {
    pub fn node(&self) -> &Node<TypeExpressionNodeValue<C>, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&TypeExpressionNodeValue<C>, &C) -> (TypeExpressionNodeValue<C2>, C2),
    ) -> TypeExpressionNode<C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        TypeExpressionNode(Node(value, node.range().clone(), ctx))
    }
}

impl TypeExpressionNode<()> {
    pub fn raw(x: TypeExpressionNodeValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }
}
