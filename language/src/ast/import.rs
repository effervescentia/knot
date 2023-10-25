use crate::Node;
use std::fmt::Debug;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportTarget {
    Named(String),
    Module,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import {
    pub source: ImportSource,
    pub path: Vec<String>,
    pub aliases: Option<Vec<(ImportTarget, Option<String>)>>,
}

impl Import {
    pub fn new(
        source: ImportSource,
        path: Vec<String>,
        aliases: Option<Vec<(ImportTarget, Option<String>)>>,
    ) -> Import {
        Import {
            source,
            path,
            aliases,
        }
    }
}

pub type ImportNodeValue = Import;

#[derive(Debug, PartialEq)]
pub struct ImportNode<R, C>(pub Node<ImportNodeValue, R, C>);

impl<R, C> ImportNode<R, C>
where
    R: Clone,
{
    pub fn node(&self) -> &Node<ImportNodeValue, R, C> {
        &self.0
    }

    pub fn map<C2>(
        &self,
        f: impl Fn(&ImportNodeValue, &C) -> (ImportNodeValue, C2),
    ) -> ImportNode<R, C2> {
        let node = self.node();
        let (value, ctx) = f(node.value(), node.context());

        ImportNode(Node(value, node.range().clone(), ctx))
    }
}

impl<R> ImportNode<R, ()> {
    pub fn raw(x: ImportNodeValue, range: R) -> Self {
        Self(Node::raw(x, range))
    }
}
