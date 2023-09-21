use std::fmt::Debug;

// TODO: rename to ImportSource
#[derive(Clone, Debug, PartialEq)]
pub enum Source {
    Root,
    Local,
    External(String),
}

// TODO: rename to ImportTarget
#[derive(Clone, Debug, PartialEq)]
pub enum Target {
    Named(String),
    Module,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Import {
    pub source: Source,
    pub path: Vec<String>,
    pub aliases: Option<Vec<(Target, Option<String>)>>,
}

impl Import {
    pub fn new(
        source: Source,
        path: Vec<String>,
        aliases: Option<Vec<(Target, Option<String>)>>,
    ) -> Import {
        Import {
            source,
            path,
            aliases,
        }
    }
}
