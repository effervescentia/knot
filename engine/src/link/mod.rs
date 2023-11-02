mod import_graph;

pub use import_graph::ImportGraph;
use lang::ast;
use std::{
    ffi::OsStr,
    path::{Path, PathBuf},
};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum LinkSource {
    Local,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Link(LinkSource, Vec<String>);

impl Link {
    pub fn to_path(&self) -> PathBuf {
        let Self(source, module_path) = self;

        match source {
            LinkSource::External(_namespace) => unimplemented!(),

            LinkSource::Local => (),
        }

        PathBuf::from_iter(module_path).with_extension("kn")
    }

    pub fn from_import<P>(file_path: P, ast::Import { source, path, .. }: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        match source {
            ast::ImportSource::Named(name) => {
                Self(LinkSource::External(name.clone()), path.clone())
            }

            ast::ImportSource::Scoped { scope, name } => Self(
                LinkSource::External(format!("@{scope}/{name}")),
                path.clone(),
            ),

            ast::ImportSource::Root => Self(LinkSource::Local, path.clone()),

            ast::ImportSource::Local => {
                let file_path = file_path.as_ref();
                let parts = file_path
                    .parent()
                    .map(Path::to_path_buf)
                    .unwrap_or_default()
                    .iter()
                    .map(|x| x.to_string_lossy().to_string())
                    .collect::<Vec<_>>();

                Self(LinkSource::Local, [parts, path.clone()].concat())
            }
        }
    }
}

impl<S> From<S> for Link
where
    S: AsRef<OsStr> + Sized,
{
    fn from(value: S) -> Self {
        let path = Path::new(&value);

        assert!(!path.is_absolute(), "must be a relative value");

        Self(
            LinkSource::Local,
            path.iter()
                .map(|x| x.to_string_lossy().to_string())
                .collect(),
        )
    }
}
