use super::Ast;
use lang::{ast, ModuleId, Namespace};
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    /// all available data can be used
    Active,

    /// must be re-analyzed before the typed AST can be used
    Stale,

    /// must be re-loaded and processed before it can be used
    Tainted,

    /// will be purged on the next incremental execution
    Evicted,
}

#[derive(Debug)]
pub struct Module {
    pub id: ModuleId,
    pub path: PathBuf,
    pub namespace: Namespace,
    pub text: String,
    pub raw: Ast<()>,
    pub typed: Option<Ast<ast::typed::Meta>>,
    pub status: Status,
}

impl Module {
    pub fn raw<T>(id: ModuleId, path: T, text: String, ast: ast::raw::Program) -> Self
    where
        T: AsRef<Path>,
    {
        Self {
            id,
            path: path.as_ref().to_path_buf(),
            namespace: Namespace::from_internal_path(path),
            text,
            raw: Ast::Program(ast),
            typed: None,
            status: Status::Active,
        }
    }

    pub fn typed(&self, typed: Ast<ast::typed::Meta>) -> Self {
        Self {
            id: self.id,
            path: self.path.clone(),
            namespace: self.namespace.clone(),
            text: self.text.clone(),
            raw: self.raw.clone(),
            typed: Some(typed),
            status: self.status,
        }
    }
}
