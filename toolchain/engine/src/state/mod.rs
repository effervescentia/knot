mod analyzed;
mod ast;
mod base;
mod linked;
mod parsed;
mod traverse;

use crate::{
    report::{Enrich, Report},
    ConfigurationError, IntoResult, Link, Result,
};
pub use analyzed::Analyzed;
pub use ast::Ast;
pub use base::Base;
use kore::internal::{self, PlatformLibrary};
use lang::NamespaceId;
pub use linked::Linked;
pub use parsed::Parsed;
use std::{fmt::Debug, path::Path};
use traverse::{DynamicVisitor, StaticVisitor};
pub use traverse::{Traverse, Visitor};

pub trait IsVerbose {
    fn is_verbose(&self) -> bool;
}

pub trait ToLibraries {
    fn to_libraries(&self) -> Vec<(internal::Library, &str)> {
        vec![]
    }
}

#[derive(Clone)]
pub struct WithLibraries<State, Library> {
    pub state: State,
    pub libraries: Vec<Library>,
}

impl<State, Library> ToLibraries for WithLibraries<State, Library>
where
    Library: Clone + PlatformLibrary,
{
    fn to_libraries(&self) -> Vec<(internal::Library, &str)> {
        self.libraries
            .iter()
            .map(|x| ((*x).into(), x.text()))
            .collect()
    }
}

impl<State, Library> Traverse for WithLibraries<State, Library>
where
    State: Traverse,
{
    type Visitor = State::Visitor;

    fn traverse(&self) -> Self::Visitor {
        self.state.traverse()
    }
}

impl<State, Library> IsVerbose for WithLibraries<State, Library>
where
    State: IsVerbose,
{
    fn is_verbose(&self) -> bool {
        self.state.is_verbose()
    }
}

impl<State, Library> Enrich for WithLibraries<State, Library> where State: Enrich {}

#[derive(Clone, Debug)]
pub struct Module<T> {
    pub id: NamespaceId,
    pub text: String,
    pub ast: Ast<T>,
}

impl<T> Module<T> {
    pub const fn new(id: NamespaceId, text: String, ast: Ast<T>) -> Self {
        Self { id, text, ast }
    }
}

#[derive(Clone)]
pub struct FromEntry {
    pub entry: Link,
    pub verbose: bool,
}

impl Enrich for FromEntry {}

impl ToLibraries for FromEntry {}

impl IntoResult for FromEntry {
    type Value = Self;

    fn into_result(self) -> Result<Self> {
        Ok(self)
    }
}

impl Traverse for FromEntry {
    type Visitor = DynamicVisitor;

    fn traverse(&self) -> Self::Visitor {
        DynamicVisitor::new(self.entry.clone())
    }
}

impl IsVerbose for FromEntry {
    fn is_verbose(&self) -> bool {
        self.verbose
    }
}

#[derive(Clone)]
pub struct FromPaths {
    pub paths: Vec<Link>,
    pub verbose: bool,
}

impl Enrich for FromPaths {}

impl ToLibraries for FromPaths {}

impl IntoResult for FromPaths {
    type Value = Self;

    fn into_result(self) -> Result<Self> {
        Ok(self)
    }
}

impl Traverse for FromPaths {
    type Visitor = StaticVisitor;

    fn traverse(&self) -> Self::Visitor {
        StaticVisitor::new(self.paths.clone())
    }
}

impl IsVerbose for FromPaths {
    fn is_verbose(&self) -> bool {
        self.verbose
    }
}

pub struct FromGlob<'a> {
    pub dir: &'a Path,
    pub glob: &'a str,
    pub verbose: bool,
}

impl<'a> FromGlob<'a> {
    pub fn to_paths(&'a self) -> Result<FromPaths> {
        let FromGlob { dir, glob, verbose } = self;

        match glob::glob(&[dir.to_string_lossy().to_string().as_str(), glob].join("/")) {
            Ok(x) => {
                let (paths, errors) = x.fold((vec![], vec![]), |(mut paths, mut errors), x| {
                    match x {
                        Ok(path) => match path.strip_prefix(dir) {
                            Ok(x) => paths.push(x.to_path_buf()),
                            Err(_) => errors.push(format!(
                                "failed to strip prefix '{}' from path '{}'",
                                dir.display(),
                                path.display()
                            )),
                        },
                        Err(err) => {
                            errors.push(err.to_string());
                        }
                    }

                    (paths, errors)
                });

                if errors.is_empty() {
                    Ok(FromPaths {
                        paths: paths.iter().map(Link::from).collect(),
                        verbose: *verbose,
                    })
                } else {
                    Err(errors)
                }
            }

            Err(err) => Err(vec![err.to_string()]),
        }
        .map_err(|errs| Box::new(Report::Configuration(ConfigurationError::InvalidGlob(errs))))
    }
}
