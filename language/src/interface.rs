use crate::ast;
use std::path::{Path, PathBuf};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ModuleScope {
    Source,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ModuleReference(pub ModuleScope, pub Vec<String>);

impl ModuleReference {
    pub fn from_import<P>(file_path: P, ast::Import { source, path, .. }: &ast::Import) -> Self
    where
        P: AsRef<Path>,
    {
        match source {
            ast::ImportSource::Named(name) => {
                Self(ModuleScope::External(name.clone()), path.clone())
            }

            ast::ImportSource::Scoped { scope, name } => Self(
                ModuleScope::External(format!("@{scope}/{name}")),
                path.clone(),
            ),

            ast::ImportSource::Root => Self(ModuleScope::Source, path.clone()),

            ast::ImportSource::Local => {
                let file_path = file_path.as_ref();
                let parts = file_path
                    .parent()
                    .map(Path::to_path_buf)
                    .unwrap_or_default()
                    .iter()
                    .map(|x| x.to_string_lossy().to_string())
                    .collect::<Vec<_>>();

                Self(ModuleScope::Source, [parts, path.clone()].concat())
            }
        }
    }

    pub fn to_path(&self, extension: &str) -> PathBuf {
        let Self(source, module_path) = self;

        match source {
            ModuleScope::External(_namespace) => unimplemented!(),

            ModuleScope::Source => (),
        }

        PathBuf::from_iter(module_path).with_extension(extension)
    }
}
