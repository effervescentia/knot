mod import_graph;

use crate::Library;
pub use import_graph::ImportGraph;
use lang::{ast, Namespace};
use std::{
    ffi::OsStr,
    fmt::{Debug, Display},
    path::{Path, PathBuf},
};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Link(pub Namespace<Library>);

impl Link {
    #[cfg(feature = "test")]
    pub fn mock() -> Self {
        Self(Namespace::mock())
    }

    pub fn from_import<P>(file_path: P, import: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        Self(Namespace::from_import(file_path, import))
    }

    pub const fn from_library(library: &Library) -> Self {
        Self(Namespace::Library(*library))
    }

    pub const fn is_library(&self) -> bool {
        matches!(self, Self(Namespace::Library(..)))
    }

    pub const fn is_internal(&self) -> bool {
        matches!(self, Self(Namespace::Internal(..)))
    }

    pub const fn is_external(&self) -> bool {
        matches!(self, Self(Namespace::External(..)))
    }

    pub fn to_path(&self) -> PathBuf {
        self.0.to_path("kn")
    }

    pub fn to_namespace(self) -> Namespace<Library> {
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

        Self(Namespace::Internal(
            path.iter()
                .map(|x| x.to_string_lossy().to_string())
                .collect(),
        ))
    }
}

impl Display for Link {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        std::fmt::Display::fmt(&self.to_path().display(), f)
    }
}
