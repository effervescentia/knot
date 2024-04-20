mod link;
mod report;
mod resolve;
mod state;
mod validate;
mod write;

use analyze::ModuleMap;
use bimap::BiMap;
use kore::{invariant, str, Generator, Incrementor};
use lang::{ast, NamespaceId};
use link::ImportGraph;
pub use link::Link;
pub use report::{CodeFrame, Error, Reporter};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
use std::{
    collections::{HashMap, VecDeque},
    path::Path,
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Vec<Error>>;

pub struct Context<Resolver> {
    reporter: Reporter,
    resolver: Resolver,
    libraries: Vec<String>,
}

impl<R> Context<R> {
    pub fn std(reporter: Reporter, resolver: R) -> Self {
        Self {
            reporter,
            resolver,
            libraries: vec![str!("std")],
        }
    }
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

    fn to_links<U>(link: &Link, ast: &state::Ast<U>) -> Vec<Link> {
        let path = link.to_path();

        if let state::Ast::Program(program) = ast {
            program
                .imports()
                .iter()
                .map(|x| Link::from_import(&path, x.0.value()))
                .collect::<Vec<_>>()
        } else {
            vec![]
        }
    }

    fn parse_one(resolver: &mut R, link: &Link) -> Result<(String, state::Ast<()>)> {
        let path = link.to_path();

        let input = resolver
            .resolve(&path)
            .ok_or(vec![Error::ModuleNotFound(link.clone())])?;

        let (ast, _) =
            parse::program::parse(&input).map_err(|_| vec![Error::InvalidSyntax(link.clone())])?;

        Ok((input, state::Ast::Program(ast)))
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
    pub const fn new(context: Context<R>) -> Self {
        Self { context, state: () }
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
            let mut incrementor = Incrementor::default();
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

                        let next_id = NamespaceId(incrementor.increment());
                        lookup.insert(link.clone(), next_id);
                        parsed.insert(link, state::Module::new(next_id, input, ast));
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
            let mut incrementor = Incrementor::default();
            let mut parsed = HashMap::new();
            let mut lookup = BiMap::new();

            for link in links {
                match Self::parse_one(&mut context.resolver, &link) {
                    Ok((text, ast)) => {
                        let next_id = NamespaceId(incrementor.increment());
                        lookup.insert(link.clone(), next_id);
                        parsed.insert(link, state::Module::new(next_id, text, ast));
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
    S::Meta: Clone,
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&'a self) -> Writer<&ast::meta::Program<S::Meta>> {
        Writer(self.state.modules().map(|modules| {
            modules
                .filter_map(|(link, state::Module { ast, .. })| match ast {
                    state::Ast::Program(x) => Some((link.to_path(), x)),
                    state::Ast::Typings(_) => None,
                })
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
    fn get_module<'a>(
        state: &'a state::Linked,
        id: &'a NamespaceId,
    ) -> (&'a Link, &'a state::Module<()>) {
        let link = state.lookup.get_by_right(id).unwrap_or_else(|| {
            invariant!("did not find link for module with id {id} in state lookup")
        });

        (
            link,
            state
                .modules
                .get(link)
                .unwrap_or_else(|| invariant!("did not find module at link {link}")),
        )
    }

    pub fn analyze(self) -> Engine<Result<state::Analyzed>, R> {
        self.then(|state, _| {
            let mut incrementor = Incrementor::default();
            let mut analyzed = HashMap::default();
            let mut modules = ModuleMap::default();
            let ambient = HashMap::default();

            for id in state.graph.iter() {
                let (link, state::Module { id, text, ast }) = Self::get_module(&state, &id);
                let namespace = link.clone().to_namespace();
                let namespace_id = NamespaceId(incrementor.increment());
                let context = analyze::Context {
                    id: namespace_id,
                    namespace: &namespace,
                    modules: &modules,
                    ambient: &ambient,
                };

                let (typed, types) = match ast {
                    state::Ast::Program(program) => analyze::analyze(&context, program.clone())
                        .map(|(typed, types)| (state::Ast::Program(typed), types)),

                    state::Ast::Typings(program) => analyze::analyze(&context, program.clone())
                        .map(|(typed, types)| (state::Ast::Typings(typed), types)),
                }
                .unwrap_or_else(|errs| unimplemented!("need to handle errors:\n{errs:?}"));

                modules.keys.insert(namespace, namespace_id);
                modules
                    .by_key
                    .insert(namespace_id, (*typed.id(), typed.exports(), types));
                analyzed.insert(link.clone(), state::Module::new(*id, text.clone(), typed));
            }

            Ok(state::Analyzed {
                graph: state.graph,
                lookup: state.lookup,
                modules: analyzed,
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
        T: Generator<Input = ast::shape::Program>,
    {
        Writer(match &self.state {
            Ok(state) => Ok(state
                .modules
                .iter()
                .filter_map(|(key, state::Module { ast, .. })| {
                    if let state::Ast::Program(program) = ast {
                        Some(generator.generate(&key.to_path(), program.clone().to_shape()))
                    } else {
                        None
                    }
                })
                .collect()),

            Err(err) => Err(err.clone()),
        })
    }
}
