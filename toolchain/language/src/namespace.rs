use crate::ast;
#[cfg(feature = "test")]
use kore::str;
use kore::{internal, invariant, pretty::Pretty};
use std::{
    fmt::Debug,
    path::{Path, PathBuf},
    str::FromStr,
};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum Namespace {
    Library(internal::Library),
    Internal(Vec<String>),
    External(String, Vec<String>),
}

impl Namespace {
    #[cfg(feature = "test")]
    pub const MOCK: &'static Self = &Self::Internal(vec![]);

    #[cfg(feature = "test")]
    pub fn mock() -> Self {
        Self::Internal(vec![str!("mock")])
    }

    pub const fn is_library(&self) -> bool {
        matches!(self, Self::Library(_))
    }

    pub fn from_internal_path<T>(path: T) -> Self
    where
        T: AsRef<Path>,
    {
        Self::Internal(
            path.as_ref()
                .with_extension("")
                .components()
                .map(|x| x.as_os_str().to_string_lossy().to_string())
                .collect(),
        )
    }

    pub fn from_path<P>(file_path: P, source: &ast::ImportSource, path: &[String]) -> Self
    where
        P: AsRef<Path>,
    {
        match source {
            ast::ImportSource::Named(name) => {
                if let Ok(library) = internal::Library::from_str(name) {
                    Self::Library(library)
                } else {
                    Self::External(name.clone(), path.to_vec())
                }
            }

            ast::ImportSource::Scoped { scope, name } => {
                Self::External(format!("@{scope}/{name}"), path.to_vec())
            }

            ast::ImportSource::Root => Self::Internal(path.to_vec()),

            ast::ImportSource::Local => {
                let file_path = file_path.as_ref();
                let parts = file_path
                    .parent()
                    .map(Path::to_path_buf)
                    .unwrap_or_default()
                    .iter()
                    .map(|x| x.to_string_lossy().to_string())
                    .collect::<Vec<_>>();

                Self::Internal([parts, path.to_vec()].concat())
            }
        }
    }

    pub fn from_import<P>(file_path: P, ast::Import { source, path, .. }: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        Self::from_path(file_path, source, path)
    }

    pub fn to_path(&self, extension: &str) -> PathBuf {
        match &self {
            Self::Library(_) => invariant!("library cannot be converted to a path"),

            Self::External(..) => unimplemented!("{self:?}"),

            Self::Internal(path) => PathBuf::from_iter(path).with_extension(extension),
        }
    }
}

impl Pretty for Namespace {
    fn pretty(&self) -> kore::color::ColoredString {
        self.to_path("kn").pretty()
    }
}
