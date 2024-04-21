use crate::ast;
use std::path::{Path, PathBuf};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum NamespaceKind {
    Library,
    Internal,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Namespace(pub NamespaceKind, pub Vec<String>);

impl Namespace {
    #[cfg(feature = "test")]
    pub const MOCK: &Self = &Self(NamespaceKind::Internal, vec![]);

    #[cfg(feature = "test")]
    pub fn mock() -> Self {
        Self(NamespaceKind::Internal, vec![])
    }

    pub fn from_path<P>(file_path: P, source: &ast::ImportSource, path: &[String]) -> Self
    where
        P: AsRef<Path>,
    {
        match source {
            ast::ImportSource::Named(name) => {
                Self(NamespaceKind::External(name.clone()), path.to_vec())
            }

            ast::ImportSource::Scoped { scope, name } => Self(
                NamespaceKind::External(format!("@{scope}/{name}")),
                path.to_vec(),
            ),

            ast::ImportSource::Root => Self(NamespaceKind::Internal, path.to_vec()),

            ast::ImportSource::Local => {
                let file_path = file_path.as_ref();
                let parts = file_path
                    .parent()
                    .map(Path::to_path_buf)
                    .unwrap_or_default()
                    .iter()
                    .map(|x| x.to_string_lossy().to_string())
                    .collect::<Vec<_>>();

                Self(NamespaceKind::Internal, [parts, path.to_vec()].concat())
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
        let Self(kind, module_path) = self;

        match kind {
            // TODO: this should never be implemented, maybe change to an invariant
            NamespaceKind::Library => unimplemented!("{self:?}"),
            NamespaceKind::External(_namespace) => unimplemented!(),

            NamespaceKind::Internal => (),
        }

        PathBuf::from_iter(module_path).with_extension(extension)
    }
}
