mod analyzed;
mod ast;
mod base;
mod linked;
mod parsed;

use crate::{
    report::{Enrich, Report},
    ConfigurationError, IntoResult, Link, Result,
};
pub use analyzed::Analyzed;
pub use ast::Ast;
pub use base::Base;
use lang::NamespaceId;
pub use linked::Linked;
pub use parsed::Parsed;
use std::{fmt::Debug, path::Path};

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

impl IntoResult for FromEntry {
    type Value = Self;

    fn into_result(self) -> Result<Self> {
        Ok(self)
    }
}

impl Enrich for FromEntry {}

#[derive(Clone)]
pub struct FromPaths {
    pub paths: Vec<Link>,
    pub verbose: bool,
}

impl IntoResult for FromPaths {
    type Value = Self;

    fn into_result(self) -> Result<Self> {
        Ok(self)
    }
}

impl Enrich for FromPaths {}

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
