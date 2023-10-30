use super::ExpressionNode;
use crate::Node;
use std::fmt::Debug;

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
        mut fe: &mut impl FnMut(&E) -> E2,
        fk: &mut impl FnMut(&K) -> K2,
    ) -> KSX<E2, K2> {
        match self {
            Self::Text(x) => KSX::Text(x.clone()),

            Self::Inline(x) => KSX::Inline(fe(x)),

            Self::Fragment(xs) => KSX::Fragment(xs.iter().map(fk).collect()),

            Self::ClosedElement(tag, xs) => KSX::ClosedElement(
                tag.clone(),
                xs.iter()
                    .map(|(key, value)| (key.clone(), value.as_ref().map(&mut fe)))
                    .collect(),
            ),

            Self::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag.clone(),
                attributes
                    .iter()
                    .map(|(key, value)| (key.clone(), value.as_ref().map(&mut fe)))
                    .collect(),
                children.iter().map(fk).collect(),
                end_tag.clone(),
            ),
        }
    }
}

pub type KSXNodeValue<R, C> = KSX<ExpressionNode<R, C>, KSXNode<R, C>>;

#[derive(Debug, PartialEq)]
pub struct KSXNode<R, C>(pub Node<KSXNodeValue<R, C>, R, C>);

impl<R, C> KSXNode<R, C>
where
    R: Copy,
{
    pub const fn node(&self) -> &Node<KSXNodeValue<R, C>, R, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&KSXNodeValue<R, C>, &C) -> (KSXNodeValue<R, C2>, C2),
    ) -> KSXNode<R, C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        KSXNode(Node(value, *node.range(), ctx))
    }
}

impl<R> KSXNode<R, ()> {
    pub const fn raw(x: KSXNodeValue<R, ()>, range: R) -> Self {
        Self(Node::raw(x, range))
    }

    pub fn bind((x, range): (KSXNodeValue<R, ()>, R)) -> Self {
        Self::raw(x, range)
    }
}
