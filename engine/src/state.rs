use crate::{link::ImportGraph, Link, Result};
use bimap::BiMap;
use lang::{ast, CanonicalId, Identify, NamespaceId};
use std::{
    collections::HashMap,
    fmt::{Display, Pointer},
    path::{Path, PathBuf},
};

pub enum Ast<Meta> {
    Program(ast::meta::Program<Meta>),
    Typings(ast::meta::Typings<Meta>),
}

impl Ast<ast::typed::Meta> {
    pub fn id(&self) -> &CanonicalId {
        match self {
            Self::Program(x) => x.0.id(),
            Self::Typings(x) => x.0.id(),
        }
    }

    pub fn exports(&self) -> HashMap<String, CanonicalId> {
        match self {
            Self::Program(x) => x.exports(),
            Self::Typings(x) => x.exports(),
        }
    }
}

impl<Meta> Display for Ast<Meta>
where
    Meta: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Program(x) => x.fmt(f),
            Self::Typings(x) => x.fmt(f),
        }
    }
}

pub trait Modules<'a> {
    type Meta: 'a;
    type Iter: Iterator<Item = (&'a Link, &'a Module<Self::Meta>)>;

    fn modules(&'a self) -> Result<Self::Iter>;
}

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
    pub lookup: BiMap<Link, NamespaceId>,
}

impl<'a> Modules<'a> for Parsed {
    type Meta = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Parsed> {
    type Meta = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}

pub struct Linked {
    pub modules: HashMap<Link, Module<()>>,
    pub lookup: BiMap<Link, NamespaceId>,
    pub graph: ImportGraph,
}

impl<'a> Modules<'a> for Linked {
    type Meta = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Linked> {
    type Meta = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}

pub struct Analyzed {
    pub modules: HashMap<Link, Module<ast::typed::Meta>>,
    pub lookup: BiMap<Link, NamespaceId>,
    pub graph: ImportGraph,
}

impl<'a> Modules<'a> for Analyzed {
    type Meta = ast::typed::Meta;
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for Result<Analyzed> {
    type Meta = ast::typed::Meta;
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Meta>>;

    fn modules(&'a self) -> Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.clone()),
        }
    }
}
