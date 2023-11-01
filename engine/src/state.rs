use crate::{link::ImportGraph, Link, Result};
use analyze::Strong;
use bimap::BiMap;
use lang::Program;
use parse::Range;
use std::{
    collections::HashMap,
    path::{Path, PathBuf},
};

pub trait Modules<'a> {
    type Context: 'a;
    type Iter: Iterator<Item = (&'a Link, &'a Module<Self::Context>)>;

    fn modules(&'a self) -> Result<Self::Iter>;
}

pub struct Module<T> {
    pub id: usize,
    pub text: String,
    pub ast: Program<Range, T>,
}

impl<T> Module<T> {
    pub const fn new(id: usize, text: String, ast: Program<Range, T>) -> Self {
        Self { id, text, ast }
    }
}

pub struct FromEntry(pub Link);

pub struct FromGlob<'a> {
    pub dir: &'a Path,
    pub glob: &'a str,
}

impl<'a> FromGlob<'a> {
    pub fn to_paths(&'a self) -> std::result::Result<Vec<PathBuf>, Vec<String>> {
        let FromGlob { dir, glob } = self;

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
                    Ok(paths)
                } else {
                    Err(errors)
                }
            }

            Err(err) => Err(vec![err.to_string()]),
        }
    }
}

pub struct Parsed {
    pub modules: HashMap<Link, Module<()>>,
    pub lookup: BiMap<Link, usize>,
}

impl<'a> Modules<'a> for Parsed {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Parsed> {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}

pub struct Linked {
    pub modules: HashMap<Link, Module<()>>,
    pub lookup: BiMap<Link, usize>,
    pub graph: ImportGraph,
}

impl<'a> Modules<'a> for Linked {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Linked> {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}

pub struct Analyzed {
    pub modules: HashMap<Link, Module<Strong>>,
    pub lookup: BiMap<Link, usize>,
    pub graph: ImportGraph,
}

impl<'a> Modules<'a> for Analyzed {
    type Context = Strong;
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Analyzed> {
    type Context = Strong;
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}
