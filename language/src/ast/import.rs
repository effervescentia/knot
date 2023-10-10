use std::fmt::Debug;

// TODO: rename to ImportSource
#[derive(Clone, Debug, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    External(String),
}

// TODO: rename to ImportTarget
#[derive(Clone, Debug, PartialEq)]
pub enum ImportTarget {
    Named(String),
    Module,
}

#[derive(Clone, Debug, PartialEq)]
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
