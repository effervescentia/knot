use crate::common::{node::Node, range::Range};
use std::fmt::Debug;

use super::ExpressionNode;

#[derive(Clone, Debug, PartialEq)]
pub enum KSX<E, K> {
    Fragment(Vec<K>),
    OpenElement(String, Vec<(String, Option<E>)>, Vec<K>, String),
    ClosedElement(String, Vec<(String, Option<E>)>),
    Inline(E),
    Text(String),
}

impl<E, K> KSX<E, K> {
    pub fn map<E2, K2>(
        &self,
        fe: &mut impl FnMut(&E) -> E2,
        fk: &mut impl FnMut(&K) -> K2,
    ) -> KSX<E2, K2> {
        match self {
            Self::Text(x) => KSX::Text(x.clone()),

            Self::Inline(x) => KSX::Inline(fe(x)),

            Self::Fragment(xs) => KSX::Fragment(xs.iter().map(fk).collect()),

            Self::ClosedElement(tag, xs) => KSX::ClosedElement(
                tag.clone(),
                xs.iter()
                    .map(|(key, value)| (key.clone(), value.as_ref().map(|x| fe(x))))
                    .collect(),
            ),

            Self::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag.clone(),
                attributes
                    .iter()
                    .map(|(key, value)| (key.clone(), value.as_ref().map(|x| fe(x))))
                    .collect(),
                children.iter().map(fk).collect(),
                end_tag.clone(),
            ),
        }
    }
}

pub type KSXNodeValue<C> = KSX<ExpressionNode<C>, KSXNode<C>>;

#[derive(Debug, PartialEq)]
pub struct KSXNode<C>(pub Node<KSXNodeValue<C>, C>);

impl<C> KSXNode<C> {
    pub fn node(&self) -> &Node<KSXNodeValue<C>, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&KSXNodeValue<C>, &C) -> (KSXNodeValue<C2>, C2),
    ) -> KSXNode<C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        KSXNode(Node(value, node.range().clone(), ctx))
    }
}

impl KSXNode<()> {
    pub fn raw(x: KSXNodeValue<()>, range: Range) -> Self {
        Self(Node::raw(x, range))
    }

    pub fn bind((x, range): (KSXNodeValue<()>, Range)) -> Self {
        Self::raw(x, range)
    }
}
