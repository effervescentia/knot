use crate::Node;
use std::fmt::{Debug, Display, Formatter};

use super::TypedNode;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum Source {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

impl Display for Source {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Root => write!(f, "@"),

            Self::Local => write!(f, "."),

            Self::Named(name) => write!(f, "{name}"),

            Self::Scoped { scope, name } => write!(f, "@{scope}/{name}"),
        }
    }
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
enum Target<T> {
    Module(String, Option<Box<T>>),
    Destructure(Vec<(Export, Option<String>)>),
}

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
pub enum Export {
    Module,
    Named(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
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

// #[derive(Clone, Debug, Eq, Hash, PartialEq)]
// pub struct Import {
//     pub source: Source,
//     pub target: Target,
// }

impl Import {
    pub fn new(
        source: ImportSource,
        path: Vec<String>,
        aliases: Option<Vec<(ImportTarget, Option<String>)>>,
    ) -> Self {
        Self {
            source,
            path,
            aliases,
        }
    }
}

// impl Import {
//     pub fn new(source: Source, target: Target) -> Self {
//         Self { source, target }
//     }
// }

pub type ImportNodeValue = Import;

pub type ImportNode<R, C> = TypedNode<ImportNodeValue, R, C>;
