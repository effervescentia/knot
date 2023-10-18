use crate::Node;
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

pub type TypeExpressionNodeValue<R, C> = TypeExpression<TypeExpressionNode<R, C>>;

#[derive(Debug, PartialEq)]
pub struct TypeExpressionNode<R, C>(pub Node<TypeExpressionNodeValue<R, C>, R, C>);

impl<R, C> TypeExpressionNode<R, C>
where
    R: Clone,
{
    pub fn node(&self) -> &Node<TypeExpressionNodeValue<R, C>, R, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&TypeExpressionNodeValue<R, C>, &C) -> (TypeExpressionNodeValue<R, C2>, C2),
    ) -> TypeExpressionNode<R, C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        TypeExpressionNode(Node(value, node.range().clone(), ctx))
    }
}

impl<R> TypeExpressionNode<R, ()> {
    pub fn raw(x: TypeExpressionNodeValue<R, ()>, range: R) -> Self {
        Self(Node::raw(x, range))
    }
}
