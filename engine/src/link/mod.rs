mod import_graph;

pub use import_graph::ImportGraph;
use lang::{ast, Namespace, NamespaceKind};
use std::{
    ffi::OsStr,
    fmt::{Debug, Display, Formatter},
    path::{Path, PathBuf},
};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Link(Namespace);

impl Link {
    pub fn to_path(&self) -> PathBuf {
        self.0.to_path("kn")
    }

    pub fn from_import<P>(file_path: P, import: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        Self(Namespace::from_import(file_path, import))
    }

    pub fn to_namespace(self) -> Namespace {
        self.0
    }
}

impl<S> From<S> for Link
where
    S: AsRef<OsStr> + Sized,
{
    fn from(value: S) -> Self {
        let path = Path::new(&value);

        assert!(!path.is_absolute(), "must be a relative value");

        Self(Namespace(
            NamespaceKind::Internal,
            path.iter()
                .map(|x| x.to_string_lossy().to_string())
                .collect(),
        ))
    }
}

impl Display for Link {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.to_path().fmt(f)
    }
}
