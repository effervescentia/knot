use super::TypedNode;
use crate::Node;
use std::fmt::{Debug, Display, Formatter};

// pub type TargetNodeValue = Target;

// #[derive(Debug, PartialEq)]
// pub struct TargetNode<R, C>(pub Node<TargetNodeValue, R, C>);

// impl<R, C> TargetNode<R, C>
// where
//     R: Copy,
// {
//     pub const fn node(&self) -> &Node<TargetNodeValue, R, C> {
//         &self.0
//     }

//     pub fn map<C2>(
//         &self,
//         f: impl Fn(&TargetNodeValue, &C) -> (TargetNodeValue, C2),
//     ) -> TargetNode<R, C2> {
//         let node = self.node();
//         let (value, ctx) = f(node.value(), node.context());

//         TargetNode(Node(value, *node.range(), ctx))
//     }
// }

// impl<R> TargetNode<R, ()> {
//     pub const fn raw(x: TargetNodeValue, range: R) -> Self {
//         Self(Node::raw(x, range))
//     }
// }

// impl Display for Target {
//     fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
//         match self {
//             Self::Module(name, None) => write!(f, "{name}"),

//             Self::Module(name, Some(next)) => write!(f, "{name}/{next}"),

//             Self::Destructure(aliases) => write!(f, "{name}"),
//         }
//     }
// }

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

pub type ImportSourceNodeValue = ImportSource;

pub type ImportSourceNode<R, C> = TypedNode<ImportSourceNodeValue, R, C>;

// #[derive(Clone, Debug, Eq, Hash, PartialEq)]
// enum Target<T> {
//     Module(String, Option<Box<T>>),
//     Destructure(Vec<(Export, Option<String>)>),
// }

// #[derive(Clone, Debug, Eq, Hash, PartialEq)]
// pub enum Export {
//     Module,
//     Named(String),
// }

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportTarget {
    Named(String),
    Module,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import<S> {
    pub source: S,
    pub path: Vec<String>,
    pub aliases: Option<Vec<(ImportTarget, Option<String>)>>,
}

// #[derive(Clone, Debug, Eq, Hash, PartialEq)]
// pub struct Import {
//     pub source: Source,
//     pub target: Target,
// }

impl<S> Import<S> {
    pub fn new(
        source: S,
        path: Vec<String>,
        aliases: Option<Vec<(ImportTarget, Option<String>)>>,
    ) -> Self {
        Self {
            source,
            path,
            aliases,
        }
    }

    pub fn map<S2>(&self, fs: &impl Fn(&S) -> S2) -> Import<S2> {
        Import {
            source: fs(&self.source),
            path: self.path.clone(),
            aliases: self.aliases.clone(),
        }
    }
}

// impl Import {
//     pub fn new(source: Source, target: Target) -> Self {
//         Self { source, target }
//     }
// }

pub type ImportNodeValue<R, C> = Import<ImportSourceNode<R, C>>;

pub type ImportNode<R, C> = TypedNode<ImportNodeValue<R, C>, R, C>;
