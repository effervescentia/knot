mod link;
mod report;
mod resolve;
mod resource;
mod state;
mod validate;
mod write;

use analyze::ModuleMap;
use kore::{invariant, Generator, Incrementor};
use lang::{ast, Canonicalize, NamespaceId, NodeId};
pub use link::Link;
pub use report::{
    CodeFrame, ConfigurationError, EnvironmentError, ExecutionError, Report, Reporter,
};
use report::{Enrich, InternalReport};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
pub use resource::Library;
use state::FromPaths;
use std::{
    collections::{HashMap, HashSet, VecDeque},
    fmt::Display,
    ops::{Deref, DerefMut},
    path::{Path, PathBuf},
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Report>;
type InternalResult<T> = std::result::Result<T, InternalReport>;

pub trait IntoResult {
    type Value;

    fn into_result(self) -> Result<Self::Value>;
}

impl<T> IntoResult for Result<T> {
    type Value = T;

    fn into_result(self) -> Result<Self::Value> {
        self
    }
}

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

    pub fn raise<T>(&mut self, x: T) -> InternalResult<()>
    where
        T: report::Errors,
    {
        self.reporter.raise(x)
    }

    pub fn fail<T>(&mut self, x: T) -> InternalReport
    where
        T: report::Errors,
    {
        self.reporter.fail(x)
    }
}

impl<R> Context<R>
where
    R: Resolver,
{
    pub fn load_and_parse_program(
        &mut self,
        link: &Link,
    ) -> InternalResult<(String, state::Ast<()>)> {
        let path = link.to_path();

        let input = self
            .resolver
            .resolve(&path)
            .ok_or_else(|| self.fail(ExecutionError::ModuleNotFound(link.clone())))?;

        let (ast, _) = parse::program::parse(&input)
            .map_err(|_| self.fail(ExecutionError::InvalidSyntax(link.clone())))?;

        Ok((input, state::Ast::Program(ast)))
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
    pub fn inspect<F>(self, f: F) -> Self
    where
        F: Fn(&T, &Context<R>),
    {
        if let Ok(state) = &self.state {
            f(state, &self.context);
        }

        self
    }

    fn to_writer<F, T2>(&self, f: F) -> Writer<T2>
    where
        T2: Display,
        F: Fn(&T) -> Vec<(PathBuf, T2)>,
    {
        Writer(self.state.as_ref().map(f).map_err(std::clone::Clone::clone))
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult,
    R: Resolver,
{
    pub fn into_result(self) -> Result<T::Value> {
        self.state.into_result()
    }
}

impl<T, U, R> Engine<U, R>
where
    T: Clone + Enrich,
    U: IntoResult<Value = T>,
    R: Resolver,
{
    fn then<F, T2>(self, f: F) -> Engine<Result<T2>, R>
    where
        F: Fn(T, &mut Context<R>) -> InternalResult<T2>,
    {
        let try_apply = |state, context: &mut Context<R>| {
            context.reporter.flush()?;

            let result = f(state, context)?;

            context.reporter.flush()?;

            Ok(result)
        };

        self.map(|result, context| {
            let state = result.into_result()?;

            try_apply(state.clone(), context).map_err(|err| state.enrich(err))
        })
    }
}

impl<'a, S, T, R> Engine<Result<S>, R>
where
    S: Deref<Target = state::Base<T>>,
    T: Clone,
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&'a self) -> Writer<ast::meta::Program<T>> {
        self.to_writer(|state| {
            state
                .modules()
                .filter_map(|(link, state::Module { ast, .. })| match ast {
                    state::Ast::Program(x) if link.is_internal() => {
                        Some((link.to_path(), x.clone()))
                    }

                    state::Ast::Program(_) | state::Ast::Typings(_) => None,
                })
                .collect()
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
    pub fn from_glob<'a>(
        self,
        dir: &'a Path,
        glob: &'a str,
    ) -> Engine<Result<state::FromPaths>, R> {
        self.map(|(), _| state::FromGlob { dir, glob }.to_paths())
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult<Value = state::FromEntry>,
    R: Resolver,
{
    /// starting from the entry file recursively discover and parse modules
    pub fn parse_and_discover(self) -> Engine<Result<state::Parsed>, R> {
        self.then(|state, context| {
            let mut queue = VecDeque::from_iter(vec![state.0]);
            let mut parsed = state::Parsed::default();

            for (library, link, module) in Self::parse_libraries(
                parsed.incrementor().borrow_mut().deref_mut(),
                &context.libraries,
            ) {
                parsed.register_library(library, link, module);
            }

            while let Some(link) = queue.pop_front() {
                context.load_and_parse_program(&link).map(|(text, ast)| {
                    for link in ast.to_links(&link) {
                        if !parsed.has_by_link(&link) && !queue.contains(&link) {
                            queue.push_back(link);
                        }
                    }

                    parsed.register_source(link, text, ast);
                })?;
            }

            Ok(parsed)
        })
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult<Value = state::FromPaths>,
    R: Resolver,
{
    /// parse all modules from the provided paths
    pub fn parse_all(self) -> Engine<Result<state::Parsed>, R> {
        self.then(|FromPaths(links), context| {
            let mut parsed = state::Parsed::default();

            for (library, link, module) in Self::parse_libraries(
                parsed.incrementor().borrow_mut().deref_mut(),
                &context.libraries,
            ) {
                parsed.register_library(library, link, module);
            }

            for link in links {
                context
                    .load_and_parse_program(&link)
                    .map(|(text, ast)| parsed.register_source(link, text, ast))?;
            }

            Ok(parsed)
        })
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult<Value = state::Parsed>,
    R: Resolver,
{
    pub fn link(self) -> Engine<Result<state::Linked>, R> {
        self.then(|state, context| {
            let linked = state.link_modules(context)?;

            Validator(context).validate(&state, &linked)?;

            Ok(state::Linked::new(state, linked))
        })
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult<Value = state::Linked>,
    R: Resolver,
{
    fn get_module<'a>(
        state: &'a state::Linked,
        id: &'a NamespaceId,
    ) -> (&'a Link, &'a state::Module<()>) {
        state.get_link_and_module_by_id(id).unwrap_or_else(|| {
            invariant!("did not find link for module with id {id} in state lookup")
        })
    }

    fn bind_errors(
        context: &analyze::Context,
        errors: Vec<(NodeId, analyze::Error)>,
    ) -> Vec<ExecutionError> {
        errors
            .into_iter()
            .map(|(id, err)| ExecutionError::AnalysisError(context.canonicalize(id), err))
            .collect()
    }

    pub fn analyze(self) -> Engine<Result<state::Analyzed>, R> {
        self.then(|state, context| {
            let mut analyzed = HashMap::default();
            let mut modules = ModuleMap::default();

            // TODO: abstract this so the same logic can be re-used between both libraries and source modules
            for (link, module) in state.modules().filter(|(link, _)| link.is_library()) {
                let namespace = link.clone().to_namespace();
                let analyze_context = analyze::Context {
                    id: module.id,
                    namespace: &namespace,
                    modules: &modules,
                    ambient: state.ambient(),
                };

                // TODO: see if it's possible to fail after all libraries are processed instead of immediately
                let (typed, types) = module
                    .ast
                    .analyze(&analyze_context)
                    .map_err(|errs| context.fail(Self::bind_errors(&analyze_context, errs)))?;

                modules
                    .by_key
                    .insert(module.id, (*typed.id(), typed.exports(), types));
                analyzed.insert(
                    link.clone(),
                    state::Module::new(module.id, module.text.clone(), typed),
                );
            }

            for id in state.iter_graph() {
                let (link, module) = Self::get_module(&state, &id);
                let namespace = link.clone().to_namespace();
                let analyze_context = analyze::Context {
                    id: module.id,
                    namespace: &namespace,
                    modules: &modules,
                    ambient: state.ambient(),
                };

                // TODO: see if it's possible to fail after all modules are processed instead of immediately
                let (typed, types) = module
                    .ast
                    .analyze(&analyze_context)
                    .map_err(|errs| context.fail(Self::bind_errors(&analyze_context, errs)))?;

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
        self.to_writer(|state| {
            state
                .internal_modules()
                .filter_map(|(key, state::Module { ast, .. })| {
                    if let state::Ast::Program(program) = ast {
                        Some(generator.generate(&key.to_path(), program.clone().to_shape()))
                    } else {
                        None
                    }
                })
                .collect()
        })
    }
}
