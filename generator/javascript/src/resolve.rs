use crate::{knot, Module};
use kore::{internal, invariant, str};
use std::{fmt::Write, marker::PhantomData, str::FromStr};

#[derive(Clone, Copy)]
pub struct Resolver<Library>(pub Module, PhantomData<Library>);

impl<Library> Resolver<Library> {
    pub const fn new(module: Module) -> Self {
        Self(module, PhantomData)
    }

    pub const fn module(&self) -> Module {
        self.0
    }

    #[allow(clippy::unused_self)]
    pub fn resolve(&self, root: &str, import: &knot::Import) -> (String, String)
    where
        Library: internal::PlatformLibrary,
    {
        let alias = match import {
            knot::Import {
                alias: Some(alias), ..
            } => alias,

            knot::Import {
                alias: None, path, ..
            } if !path.is_empty() => path.last().unwrap(),

            knot::Import {
                source: knot::ImportSource::Named(name),
                ..
            } => name,

            _ => invariant!("failed to get the implicit module name from import {import:?}"),
        };

        let mut namespace = match &import.source {
            knot::ImportSource::Local => str!("."),

            knot::ImportSource::Root => root.to_owned(),

            knot::ImportSource::Named(name) => {
                if let Ok(module) = internal::Library::from_str(name)
                    .map(Library::from)
                    .and_then(|x| x.module().ok_or(()))
                {
                    format!("@knot/{module}")
                } else {
                    name.to_owned()
                }
            }

            knot::ImportSource::Scoped { scope, name } => {
                format!("@{scope}/{name}")
            }
        };

        if !import.path.is_empty() {
            write!(&mut namespace, "/{}", import.path.join("/")).ok();

            if matches!(
                import.source,
                knot::ImportSource::Local | knot::ImportSource::Root
            ) {
                write!(&mut namespace, ".js").ok();
            }
        }

        (namespace, alias.to_owned())
    }
}
