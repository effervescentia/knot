mod link;
mod report;
mod resolve;
mod state;
mod validate;
mod write;

use bimap::BiMap;
use kore::Generator;
use lang::{
    ast::{self, AstNode, ToShape},
    Program,
};
use link::ImportGraph;
pub use link::Link;
use parse::Range;
pub use report::{CodeFrame, Error, Reporter};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
use std::{
    collections::{HashMap, VecDeque},
    path::Path,
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Vec<Error>>;

pub struct Context<R>
where
    R: Resolver,
{
    reporter: Reporter,
    resolver: R,
}

pub struct Engine<T, R>
where
    R: Resolver,
{
    context: Context<R>,
    state: T,
}

impl<T, R> Engine<T, R>
where
    R: Resolver,
{
    pub fn map<F, T2>(mut self, f: F) -> Engine<T2, R>
    where
        F: Fn(T, &mut Context<R>) -> T2,
    {
        let state = f(self.state, &mut self.context);

        Engine {
            context: self.context,
            state,
        }
    }

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
            .ok_or(vec![Error::ModuleNotFound(link.clone())])?;

        let (ast, _) =
            parse::parse(&input).map_err(|_| vec![Error::InvalidSyntax(link.clone())])?;

        Ok((input, ast))
    }
}

impl<T, R> Engine<Result<T>, R>
where
    R: Resolver,
{
    pub fn then<F, T2>(self, f: F) -> Engine<Result<T2>, R>
    where
        F: Fn(T, &mut Context<R>) -> Result<T2>,
    {
        self.map(|state, context| match state {
            Ok(state) => {
                context.reporter.catch()?;

                f(state, context)
            }
            Err(err) => Err(err),
        })
    }
}

impl<R> Engine<(), R>
where
    R: Resolver,
{
    pub const fn new(reporter: Reporter, resolver: R) -> Self {
        Self {
            context: Context { reporter, resolver },
            state: (),
        }
    }

    /// load a module tree from a single entry point
    pub fn from_entry(self, entry: &Path) -> Engine<state::FromEntry, R> {
        assert!(
            !entry.is_absolute(),
            "entry must be relative to the source directory"
        );

        self.map(|(), _| state::FromEntry(Link::from(&entry)))
    }

    /// load all modules that match a glob
    pub fn from_glob<'a>(self, dir: &'a Path, glob: &'a str) -> Engine<state::FromGlob<'a>, R> {
        self.map(|(), _| state::FromGlob { dir, glob })
    }
}

impl<R> Engine<state::FromEntry, R>
where
    R: Resolver,
{
    /// starting from the entry file recursively discover and parse modules
    pub fn parse_and_load(self) -> Engine<Result<state::Parsed>, R> {
        self.map(|state, context| {
            let mut next_id: usize = 0;
            let mut queue = VecDeque::from_iter(vec![state.0]);
            let mut parsed = HashMap::new();
            let mut lookup = BiMap::new();

            while let Some(link) = queue.pop_front() {
                match Self::parse_one(&mut context.resolver, &link) {
                    Ok((input, ast)) => {
                        let links = Self::to_links(&link, &ast);

                        for x in links {
                            if !parsed.contains_key(&x) && !queue.contains(&x) {
                                queue.push_back(x);
                            }
                        }

                        lookup.insert(link.clone(), next_id);
                        parsed.insert(link, state::Module::new(next_id, input, ast));

                        next_id += 1;
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(state::Parsed {
                modules: parsed,
                lookup,
            })
        })
    }
}

impl<'a, R> Engine<state::FromGlob<'a>, R>
where
    R: Resolver,
{
    /// parse all modules that match the glob
    pub fn parse(self) -> Engine<Result<state::Parsed>, R> {
        self.map(move |state, context| {
            let links = state
                .to_paths()
                .map_err(|errs| vec![Error::InvalidGlob(errs)])?
                .iter()
                .map(Link::from)
                .collect::<Vec<_>>();
            let mut next_id: usize = 0;
            let mut parsed = HashMap::new();
            let mut lookup = BiMap::new();

            for link in links {
                match Self::parse_one(&mut context.resolver, &link) {
                    Ok((text, ast)) => {
                        lookup.insert(link.clone(), next_id);
                        parsed.insert(link, state::Module::new(next_id, text, ast));

                        next_id += 1;
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(state::Parsed {
                modules: parsed,
                lookup,
            })
        })
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

    pub fn link(self) -> Engine<Result<state::Linked>, R> {
        self.then(|state, context| {
            let graph = Self::populate_graph(&state.modules);
            let linked = state.modules.iter().fold(graph, |mut acc, (link, module)| {
                let links = Self::to_links(link, &module.ast);

                for x in &links {
                    if let Some(x) = state.lookup.get_by_left(x) {
                        acc.add_edge(&module.id, x).ok();
                    } else {
                        context
                            .reporter
                            .report(Error::UnregisteredModule(x.clone()));
                    }
                }

                acc
            });

            context.reporter.catch_early()?;

            let validator = Validator(&state);
            context
                .reporter
                .raise(validator.assert_no_import_cycles(&linked))?;

            Ok(state::Linked {
                graph: linked,
                lookup: state.lookup,
                modules: state.modules,
            })
        })
    }
}

impl<R> Engine<Result<state::Linked>, R>
where
    R: Resolver,
{
    pub fn analyze(self) -> Engine<Result<state::Analyzed>, R> {
        self.then(|state, _| {
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
