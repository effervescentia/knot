use crate::{link::ImportGraph, Link, Result};
use bimap::BiMap;
use lang::{ast, CanonicalId, Identify, NamespaceId};
use std::{
    collections::HashMap,
    fmt::{Debug, Display, Pointer},
    path::{Path, PathBuf},
};

#[derive(Debug)]
pub enum Ast<Meta> {
    Program(ast::meta::Program<Meta>),
    Typings(ast::meta::Typings<Meta>),
}

impl<Meta> Ast<Meta>
where
    Meta: Clone,
{
    pub fn analyze(
        &self,
        context: &analyze::Context,
    ) -> analyze::Result<(Ast<ast::typed::Meta>, analyze::TypeMap)> {
        match self {
            Self::Program(program) => analyze::analyze(context, program.clone())
                .map(|(typed, types)| (Ast::Program(typed), types)),

            Self::Typings(typings) => analyze::analyze(context, typings.clone())
                .map(|(typed, types)| (Ast::Typings(typed), types)),
        }
    }
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
    type Meta: Debug + 'a;
    type Iter: Iterator<Item = (&'a Link, &'a Module<Self::Meta>)>;

    fn modules(&'a self) -> Result<Self::Iter>;
}

// pub trait InternalModules<'a, T> {
//     fn internal_modules(&'a self) -> Vec<Item = (&Link, &Module<()>)>;
// }

#[derive(Debug)]
pub struct Module<T>
where
    T: Debug,
{
    pub id: NamespaceId,
    pub text: String,
    pub ast: Ast<T>,
}

impl<T> Module<T>
where
    T: Debug,
{
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
    pub ambient: analyze::AmbientMap,
}

impl Parsed {
    pub fn to_import_graph(&self) -> ImportGraph {
        self.internal_modules()
            .fold(ImportGraph::new(), |mut graph, (_, x)| {
                graph.add_node(x.id);
                graph
            })
    }

    pub fn internal_modules(&self) -> impl Iterator<Item = (&Link, &Module<()>)> {
        self.modules
            .iter()
            .filter_map(|(link, module)| link.is_internal().then_some((link, module)))
    }
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
    modules: HashMap<Link, Module<()>>,
    pub lookup: BiMap<Link, NamespaceId>,
    pub ambient: analyze::AmbientMap,
    pub graph: ImportGraph,
}

impl Linked {
    pub fn new(state: Parsed, graph: ImportGraph) -> Self {
        Self {
            graph,
            lookup: state.lookup,
            ambient: state.ambient,
            modules: state.modules,
        }
    }

    pub fn get_module(&self, link: &Link) -> Option<&Module<()>> {
        self.modules.get(link)
    }
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
    modules: HashMap<Link, Module<ast::typed::Meta>>,
    pub lookup: BiMap<Link, NamespaceId>,
    pub ambient: analyze::AmbientMap,
    pub graph: ImportGraph,
}

impl Analyzed {
    pub fn new(state: Linked, modules: HashMap<Link, Module<ast::typed::Meta>>) -> Self {
        Self {
            modules,
            graph: state.graph,
            lookup: state.lookup,
            ambient: state.ambient,
        }
    }

    pub fn internal_modules(&self) -> impl Iterator<Item = (&Link, &Module<ast::typed::Meta>)> {
        self.modules
            .iter()
            .filter_map(|(link, module)| link.is_internal().then_some((link, module)))
    }
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
