mod link;
mod report;
mod resolve;
mod resource;
mod state;
mod validate;
mod write;

use analyze::ModuleMap;
use bimap::BiMap;
use kore::{invariant, Generator, Incrementor};
use lang::{ast, NamespaceId};
pub use link::Link;
pub use report::{CodeFrame, Error, Reporter};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
pub use resource::Library;
use state::Modules;
use std::{
    collections::{HashMap, HashSet, VecDeque},
    path::Path,
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Vec<Error>>;

pub struct Context<Resolver> {
    reporter: Reporter,
    resolver: Resolver,
    libraries: HashSet<Library>,
}

impl<R> Context<R> {
    pub fn std(reporter: Reporter, resolver: R) -> Self {
        Self {
            reporter,
            resolver,
            libraries: HashSet::from_iter(vec![Library::Std, Library::Html]),
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

    fn load_and_parse_program(resolver: &mut R, link: &Link) -> Result<(String, state::Ast<()>)> {
        let path = link.to_path();

        let input = resolver
            .resolve(&path)
            .ok_or(vec![Error::ModuleNotFound(link.clone())])?;

        let (ast, _) =
            parse::program::parse(&input).map_err(|_| vec![Error::InvalidSyntax(link.clone())])?;

        Ok((input, state::Ast::Program(ast)))
    }

    fn parse_library(library: &Library) -> (String, state::Ast<()>) {
        let input = library.resolve();
        let (ast, _) = parse::typings::parse(input)
            .unwrap_or_else(|_| invariant!("failed to parse library {library:?}"));

        (input.to_owned(), state::Ast::Typings(ast))
    }

    fn parse_libraries<'a>(
        incrementor: &'a mut Incrementor,
        libraries: &'a HashSet<Library>,
    ) -> impl Iterator<Item = (Library, Link, state::Module<()>)> + 'a {
        libraries.iter().map(|library| {
            let (input, ast) = Self::parse_library(library);

            (
                *library,
                Link::from_library(library),
                state::Module::new(NamespaceId(incrementor.increment()), input, ast),
            )
        })
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

    pub fn inspect<F>(self, f: F) -> Self
    where
        F: Fn(&T, &Context<R>),
    {
        if let Ok(state) = &self.state {
            f(state, &self.context);
        }

        self
    }

    pub fn into_result(self) -> Result<T> {
        self.state
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
    pub fn parse_and_discover(self) -> Engine<Result<state::Parsed>, R> {
        self.map(|state, context| {
            let mut incrementor = Incrementor::default();
            let mut queue = VecDeque::from_iter(vec![state.0]);
            let mut ambient = HashMap::new();
            let mut modules = HashMap::new();
            let mut lookup = BiMap::new();

            for (library, link, module) in
                Self::parse_libraries(&mut incrementor, &context.libraries)
            {
                if let Some(ambient_scope) = library.to_ambient_scope() {
                    ambient.insert(ambient_scope, module.id);
                }
                modules.insert(link, module);
            }

            while let Some(link) = queue.pop_front() {
                match Self::load_and_parse_program(&mut context.resolver, &link) {
                    Ok((input, ast)) => {
                        let links = Self::to_links(&link, &ast);

                        for link in links {
                            if !modules.contains_key(&link) && !queue.contains(&link) {
                                queue.push_back(link);
                            }
                        }

                        let namespace_id = NamespaceId(incrementor.increment());
                        lookup.insert(link.clone(), namespace_id);
                        modules.insert(link, state::Module::new(namespace_id, input, ast));
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(state::Parsed {
                modules,
                lookup,
                ambient,
            })
        })
    }
}

impl<'a, R> Engine<state::FromGlob<'a>, R>
where
    R: Resolver,
{
    /// parse all modules that match the glob
    pub fn parse_matched(self) -> Engine<Result<state::Parsed>, R> {
        self.map(move |state, context| {
            let links = state
                .to_paths()
                .map_err(|errs| vec![Error::InvalidGlob(errs)])?
                .iter()
                .map(Link::from)
                .collect::<Vec<_>>();
            let mut incrementor = Incrementor::default();
            let mut modules = HashMap::new();
            let mut ambient = HashMap::new();
            let mut lookup = BiMap::new();

            for (library, link, module) in
                Self::parse_libraries(&mut incrementor, &context.libraries)
            {
                if let Some(ambient_scope) = library.to_ambient_scope() {
                    ambient.insert(ambient_scope, module.id);
                }
                modules.insert(link, module);
            }

            for link in links {
                match Self::load_and_parse_program(&mut context.resolver, &link) {
                    Ok((text, ast)) => {
                        let namespace_id = NamespaceId(incrementor.increment());
                        lookup.insert(link.clone(), namespace_id);
                        modules.insert(link, state::Module::new(namespace_id, text, ast));
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(state::Parsed {
                modules,
                lookup,
                ambient,
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
                    state::Ast::Program(x) if link.is_internal() => Some((link.to_path(), x)),

                    state::Ast::Program(_) | state::Ast::Typings(_) => None,
                })
                .collect::<Vec<_>>()
        }))
    }
}

impl<R> Engine<Result<state::Parsed>, R>
where
    R: Resolver,
{
    pub fn link(self) -> Engine<Result<state::Linked>, R> {
        self.then(|state, context| {
            let linked = state.internal_modules().fold(
                state.to_import_graph(),
                |mut acc, (link, module)| {
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
                },
            );

            context.reporter.catch_early()?;

            let validator = Validator(&state);
            context
                .reporter
                .raise(validator.assert_no_import_cycles(&linked))?;

            Ok(state::Linked::new(state, linked))
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
                .get_module(link)
                .unwrap_or_else(|| invariant!("did not find module at link {link}")),
        )
    }

    pub fn analyze(self) -> Engine<Result<state::Analyzed>, R> {
        self.then(|state, _| {
            let mut analyzed = HashMap::default();
            let mut modules = ModuleMap::default();

            // TODO: abstract this so the same logic can be re-used between both libraries and source modules
            for (link, module) in state.modules()?.filter(|(link, _)| link.is_library()) {
                let namespace = link.clone().to_namespace();
                let context = analyze::Context {
                    id: module.id,
                    namespace: &namespace,
                    modules: &modules,
                    ambient: &state.ambient,
                };

                let (typed, types) = module
                    .ast
                    .analyze(&context)
                    .unwrap_or_else(|errs| unimplemented!("need to handle errors:\n{errs:?}"));

                modules
                    .by_key
                    .insert(module.id, (*typed.id(), typed.exports(), types));
                analyzed.insert(
                    link.clone(),
                    state::Module::new(module.id, module.text.clone(), typed),
                );
            }

            for id in state.graph.iter() {
                let (link, module) = Self::get_module(&state, &id);
                let namespace = link.clone().to_namespace();
                let context = analyze::Context {
                    id: module.id,
                    namespace: &namespace,
                    modules: &modules,
                    ambient: &state.ambient,
                };

                let (typed, types) = module
                    .ast
                    .analyze(&context)
                    .unwrap_or_else(|errs| unimplemented!("need to handle errors:\n{errs:?}"));

                modules.keys.insert(namespace, module.id);
                modules
                    .by_key
                    .insert(module.id, (*typed.id(), typed.exports(), types));
                analyzed.insert(
                    link.clone(),
                    state::Module::new(module.id, module.text.clone(), typed),
                );
            }

            Ok(state::Analyzed::new(state, analyzed))
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
                .internal_modules()
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
