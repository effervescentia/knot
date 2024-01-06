use super::TypedNode;
use std::fmt::Debug;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import {
    pub source: ImportSource,
    pub path: Vec<String>,
    pub alias: Option<String>,
}

impl Import {
    pub fn new(source: ImportSource, path: Vec<String>, alias: Option<String>) -> Self {
        Self {
            source,
            path,
            alias,
        }
    }
}

pub type ImportNodeValue = Import;

pub type ImportNode<R, C> = TypedNode<ImportNodeValue, R, C>;
