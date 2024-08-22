use crate::Module;
use kore::{invariant, str};
use lang::ast;
use std::{
    fmt::{Display, Write},
    marker::PhantomData,
    str::FromStr,
};

#[derive(Clone, Copy)]
pub struct ImportResolver<Library>(pub Module, PhantomData<Library>);

impl<Library> ImportResolver<Library> {
    pub const fn new(module: Module) -> Self {
        Self(module, PhantomData)
    }

    pub const fn module(&self) -> Module {
        self.0
    }

    #[allow(clippy::unused_self)]
    pub fn resolve(&self, root: &str, import: &ast::Import) -> (String, String)
    where
        Library: FromStr + Display,
    {
        let alias = match import {
            ast::Import {
                alias: Some(alias), ..
            } => alias,

            ast::Import {
                alias: None, path, ..
            } if !path.is_empty() => path.last().unwrap(),

            ast::Import {
                source: ast::ImportSource::Named(name),
                ..
            } => name,

            _ => invariant!("failed to get the implicit module name from import {import:?}"),
        };

        let mut namespace = match &import.source {
            ast::ImportSource::Local => str!("."),

            ast::ImportSource::Root => root.to_owned(),

            ast::ImportSource::Named(name) => {
                if let Ok(library) = Library::from_str(name) {
                    format!("@knot/{library}")
                } else {
                    name.to_owned()
                }
            }

            ast::ImportSource::Scoped { scope, name } => {
                format!("@{scope}/{name}")
            }
        };

        if !import.path.is_empty() {
            write!(&mut namespace, "/{}", import.path.join("/")).ok();

            if matches!(
                import.source,
                ast::ImportSource::Local | ast::ImportSource::Root
            ) {
                write!(&mut namespace, ".js").ok();
            }
        }

        (namespace, alias.to_owned())
    }
}
