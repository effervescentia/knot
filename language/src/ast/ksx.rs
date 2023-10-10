use crate::common::{node::Node, position::Decrement, range::Range};
use combine::Stream;
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

pub type KSXNodeValue<T, C> = KSX<ExpressionNode<T, C>, KSXNode<T, C>>;

#[derive(Debug, PartialEq)]
pub struct KSXNode<T, C>(pub Node<KSXNodeValue<T, C>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> KSXNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<KSXNodeValue<T, C>, T, C> {
        &self.0
    }

    pub fn map<R>(
        &self,
        f: impl Fn(&KSXNodeValue<T, C>, &C) -> (KSXNodeValue<T, R>, R),
    ) -> KSXNode<T, R> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        KSXNode(Node(value, node.range().clone(), ctx))
    }
}

impl<T> KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: KSXNodeValue<T, ()>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }

    pub fn bind((x, range): (KSXNodeValue<T, ()>, Range<T>)) -> Self {
        Self::raw(x, range)
    }
}
