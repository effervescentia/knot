mod link;
mod report;
mod resolve;
mod state;
mod write;

use bimap::BiMap;
use kore::Generator;
use lang::{
    ast::{self, ToShape},
    Program,
};
use link::ImportGraph;
pub use link::Link;
use parse::Range;
pub use report::{CodeFrame, Error};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
use std::{
    collections::{HashMap, VecDeque},
    path::{Path, PathBuf},
};
use write::Writer;

pub type Result<T> = std::result::Result<T, Vec<Error>>;

pub struct Engine<T, R>
where
    R: Resolver,
{
    resolver: R,
    state: T,
}

impl<T, R> Engine<T, R>
where
    R: Resolver,
{
    fn to_links<U>(link: &Link, ast: &Program<Range, U>) -> Vec<Link> {
        let path = link.to_path();

        ast.imports()
            .iter()
            .map(|x| Link::from_import(&path, x.node().value()))
            .collect::<Vec<_>>()
    }

    fn parse_one(resolver: &mut R, link: &Link) -> Result<(String, Program<Range, ()>)> {
        let path = link.to_path();

        let input = resolver
            .resolve(&path)
            .ok_or(vec![Error::ModuleNotFound(link.to_owned())])?;

        let (ast, _) =
            parse::parse(&input).map_err(|_| vec![Error::InvalidSyntax(link.to_owned())])?;

        Ok((input, ast))
    }
}

impl<T, R> Engine<Result<T>, R>
where
    R: Resolver,
{
    pub fn map<F, T2>(self, f: F) -> Engine<Result<T2>, R>
    where
        F: Fn(T) -> Result<T2>,
    {
        match self.state {
            Ok(state) => Engine {
                resolver: self.resolver,
                state: f(state),
            },

            Err(err) => Engine {
                resolver: self.resolver,
                state: Err(err),
            },
        }
    }

    pub fn unwrap(self) -> Engine<T, R> {
        Engine {
            resolver: self.resolver,
            state: self.state.unwrap(),
        }
    }

    pub fn expect(self, message: &str) -> Engine<T, R> {
        Engine {
            resolver: self.resolver,
            state: self.state.expect(message),
        }
    }
}

impl<R> Engine<(), R>
where
    R: Resolver,
{
    pub fn new(resolver: R) -> Self {
        Self {
            resolver,
            state: (),
        }
    }

    /// load a module tree from a single entry point
    pub fn from_entry(self, entry: &Path) -> Engine<state::FromEntry, R> {
        if entry.is_absolute() {
            panic!("entry must be relative to the source directory");
        }

        Engine {
            resolver: self.resolver,
            state: state::FromEntry(Link::from(&entry)),
        }
    }

    /// load all modules that match a glob
    pub fn from_glob<'a>(self, dir: &'a Path, glob: &'a str) -> Engine<state::FromGlob<'a>, R> {
        Engine {
            resolver: self.resolver,
            state: state::FromGlob { dir, glob },
        }
    }
}

impl<R> Engine<state::FromEntry, R>
where
    R: Resolver,
{
    /// starting from the entry file recursively discover and parse modules
    pub fn parse_and_load(mut self) -> Engine<Result<state::Parsed>, R> {
        let mut next_id: usize = 0;
        let mut queue = VecDeque::from_iter(vec![self.state.0]);
        let mut parsed = HashMap::new();
        let mut lookup = BiMap::new();
        let mut all_errors = vec![];

        while let Some(link) = queue.pop_front() {
            match Self::parse_one(&mut self.resolver, &link) {
                Ok((input, ast)) => {
                    let links = Self::to_links(&link, &ast);

                    links.into_iter().for_each(|x| {
                        if !parsed.contains_key(&x) && !queue.contains(&x) {
                            queue.push_back(x);
                        }
                    });

                    lookup.insert(link.clone(), next_id);
                    parsed.insert(link, state::Module::new(next_id, input, ast));

                    next_id += 1;
                }

                Err(errs) => all_errors.extend(errs),
            }
        }

        Engine {
            resolver: self.resolver,
            state: if all_errors.is_empty() {
                Ok(state::Parsed {
                    modules: parsed,
                    lookup,
                })
            } else {
                Err(all_errors)
            },
        }
    }
}

impl<'a, R> Engine<state::FromGlob<'a>, R>
where
    R: Resolver,
{
    fn glob(&self) -> Vec<PathBuf> {
        let state::FromGlob { dir, glob } = self.state;

        glob::glob(
            &[
                dir.to_str().expect("failed to convert directory to string"),
                glob,
            ]
            .join("/"),
        )
        .expect("failed to compile glob")
        .map(|x| x.expect("failed to apply glob"))
        .map(|x| {
            x.strip_prefix(dir)
                .expect("failed to strip prefix from path")
                .to_path_buf()
        })
        .collect::<Vec<_>>()
    }

    /// parse all modules that match the glob
    pub fn parse(mut self) -> Engine<Result<state::Parsed>, R> {
        let mut next_id: usize = 0;
        let mut parsed = HashMap::new();
        let mut lookup = BiMap::new();
        let mut errors = vec![];
        let links = self.glob().iter().map(Link::from).collect::<Vec<_>>();

        println!("found some {links:?}");

        for link in links {
            match Self::parse_one(&mut self.resolver, &link) {
                Ok((text, ast)) => {
                    lookup.insert(link.clone(), next_id);
                    parsed.insert(link, state::Module::new(next_id, text, ast));

                    next_id += 1;
                }

                Err(errs) => errors.extend(errs),
            }
        }

        Engine {
            resolver: self.resolver,
            state: if errors.is_empty() {
                Ok(state::Parsed {
                    modules: parsed,
                    lookup,
                })
            } else {
                Err(errors)
            },
        }
    }
}

impl<'a, S, R> Engine<S, R>
where
    S: state::Modules<'a>,
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&'a self) -> Writer<&Program<Range, S::Context>> {
        Writer(self.state.modules().map(|modules| {
            modules
                .map(|(link, state::Module { ast, .. })| (link.to_path(), ast))
                .collect::<Vec<_>>()
        }))
    }
}

impl<R> Engine<Result<state::Parsed>, R>
where
    R: Resolver,
{
    fn populate_graph(modules: &HashMap<Link, state::Module<()>>) -> ImportGraph {
        modules.values().fold(ImportGraph::new(), |mut graph, x| {
            graph.add_node(x.id);
            graph
        })
    }

    fn find_import_cycles(state: &state::Parsed, graph: &ImportGraph) -> Vec<Error> {
        let cycles = graph.cycles();

        cycles
            .into_iter()
            .map(|x| {
                Error::ImportCycle(
                    x.to_vec()
                        .iter()
                        .map(|x| {
                            state
                                .lookup
                                .get_by_right(x)
                                .expect(&format!("failed to map module id {x} to link"))
                                .clone()
                        })
                        .collect::<Vec<_>>(),
                )
            })
            .collect()
    }

    pub fn link(self) -> Engine<Result<state::Linked>, R> {
        self.map(|state| {
            let mut errors = vec![];
            let graph = Self::populate_graph(&state.modules);
            let linked = state.modules.iter().fold(graph, |mut acc, (link, module)| {
                let links = Self::to_links(link, &module.ast);

                for x in &links {
                    if let Some(x) = state.lookup.get_by_left(x) {
                        acc.add_edge(&module.id, x).ok();
                    } else {
                        errors.push(Error::UnregisteredModule(x.clone()));
                    }
                }

                acc
            });

            if !errors.is_empty() {
                return Err(errors);
            }

            if linked.is_cyclic() {
                let errors = Self::find_import_cycles(&state, &linked);

                Err(errors)
            } else {
                Ok(state::Linked {
                    graph: linked,
                    lookup: state.lookup,
                    modules: state.modules,
                })
            }
        })
    }
}

impl<R> Engine<Result<state::Linked>, R>
where
    R: Resolver,
{
    pub fn analyze(self) -> Engine<Result<state::Analyzed>, R> {
        self.map(|state| {
            let analyzed = state
                .modules
                .into_iter()
                .map(|(key, state::Module { id, text, ast })| {
                    let typed = analyze::analyze(&ast);

                    (key, state::Module::new(id, text, typed))
                })
                .collect::<Vec<_>>();

            Ok(state::Analyzed {
                graph: state.graph,
                lookup: state.lookup,
                modules: HashMap::from_iter(analyzed),
            })
        })
    }
}

impl<R> Engine<Result<state::Analyzed>, R>
where
    R: Resolver,
{
    /// generate output files using the provided generator
    pub fn generate<T>(&self, generator: &T) -> Writer<T::Output>
    where
        T: Generator<Input = ast::ProgramShape>,
    {
        Writer(match &self.state {
            Ok(state) => Ok(state
                .modules
                .iter()
                .map(|(key, state::Module { ast, .. })| {
                    generator.generate(&key.to_path(), ast.to_shape())
                })
                .collect()),

            Err(err) => Err(err.clone()),
        })
    }
}
