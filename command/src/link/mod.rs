use std::path::{Path, PathBuf};

use lang::ast;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum LinkSource {
    Internal,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Link(LinkSource, Vec<String>);

impl Link {
    pub fn from_path<P>(path: P) -> Self
    where
        P: AsRef<Path>,
    {
        if path.as_ref().is_absolute() {
            panic!("must be a relative path");
        }

        Self(
            LinkSource::Internal,
            path.as_ref()
                .iter()
                .map(|x| x.to_string_lossy().to_string())
                .collect(),
        )
    }

    pub fn to_path(&self) -> PathBuf {
        let Self(source, module_path) = self;

        match source {
            LinkSource::External(_namespace) => todo!(),

            LinkSource::Internal => (),
        }

        PathBuf::from_iter(module_path).with_extension("kn")
    }

    pub fn from_import<P>(file_path: P, ast::Import { source, path, .. }: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        match source {
            ast::ImportSource::External(ns) => Self(LinkSource::External(ns.clone()), path.clone()),

            ast::ImportSource::Root => Self(LinkSource::Internal, path.clone()),

            ast::ImportSource::Local => {
                let relative_path = file_path
                    .as_ref()
                    .parent()
                    .unwrap()
                    .iter()
                    .map(|x| x.to_str().unwrap().to_string())
                    .collect::<Vec<_>>();

                Self(LinkSource::Internal, [relative_path, path.clone()].concat())
            }
        }
    }
}
