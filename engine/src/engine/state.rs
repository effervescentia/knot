use crate::link::{ImportGraph, Link};
use analyze::Strong;
use bimap::BiMap;
use lang::Program;
use parse::Range;
use std::{collections::HashMap, path::Path};

pub trait Modules<'a> {
    type Context: 'a;
    type Iter: Iterator<Item = (&'a Link, &'a Module<Self::Context>)>;

    fn modules(&'a self) -> super::Result<Self::Iter>;
}

pub struct Module<T> {
    pub id: usize,
    pub text: String,
    pub ast: Program<Range, T>,
}

impl<T> Module<T> {
    pub fn new(id: usize, text: String, ast: Program<Range, T>) -> Self {
        Self { id, text, ast }
    }
}

pub struct FromEntry(pub Link);

pub struct FromGlob<'a> {
    pub dir: &'a Path,
    pub glob: &'a str,
}

pub struct Parsed {
    pub modules: HashMap<Link, Module<()>>,
    pub lookup: BiMap<Link, usize>,
}

impl<'a> Modules<'a> for Parsed {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> super::Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for super::Result<Parsed> {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> super::Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.to_owned()),
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

    fn modules(&'a self) -> super::Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for super::Result<Linked> {
    type Context = ();
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> super::Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.to_owned()),
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

    fn modules(&'a self) -> super::Result<Self::Iter> {
        Ok(self.modules.iter())
    }
}

impl<'a> Modules<'a> for super::Result<Analyzed> {
    type Context = Strong;
    type Iter = std::collections::hash_map::Iter<'a, Link, Module<Self::Context>>;

    fn modules(&'a self) -> super::Result<Self::Iter> {
        match self {
            Ok(x) => Ok(x.modules.iter()),
            Err(err) => Err(err.to_owned()),
        }
    }
}
