pub mod engine2;
mod link;
mod report;
mod resolve;
mod state;
mod validate;
mod write;

use analyze::ModuleMap;
use kore::{internal, invariant, Incrementor};
use lang::{ast, Canonicalize, NamespaceId, NodeId};
pub use link::Link;
use report::Enrich;
pub use report::{
    CodeFrame, ConfigurationError, EnvironmentError, ExecutionError, Report, Reporter,
};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
use state::{IsVerbose, ToLibraries, Traverse, Visitor, WithLibraries};
use std::{
    collections::HashMap,
    env::current_dir,
    fmt::Display,
    ops::Deref,
    path::{Path, PathBuf},
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Box<Report>>;

/// internal result type used to propagate errors
type Internal<T> = std::result::Result<T, Box<report::Failure>>;

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

#[derive(Clone)]
pub struct Context<Resolver> {
    reporter: Reporter,
    resolver: Resolver,
}

impl<R> Context<R> {
    pub const fn std(reporter: Reporter, resolver: R) -> Self {
        Self { reporter, resolver }
    }

    pub fn raise<T>(&mut self, x: T) -> Internal<()>
    where
        T: report::IntoErrors,
    {
        self.reporter.raise(x)
    }

    pub fn fail<T>(&mut self, x: T) -> report::Failure
    where
        T: report::IntoErrors,
    {
        self.reporter.fail(x)
    }
}

impl<R> Context<R>
where
    R: Resolver,
{
    pub fn load_and_parse_program(&mut self, link: &Link) -> Internal<(String, state::Ast<()>)> {
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

#[derive(Clone)]
pub struct Engine<T, R> {
    root_dir: String,
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
            root_dir: self.root_dir,
            context: self.context,
            state,
        }
    }

    fn parse_library(library: &internal::Library, text: &str) -> (String, state::Ast<()>) {
        let (ast, _) = parse::typings::parse(text)
            .unwrap_or_else(|_| invariant!("failed to parse library {library:?}"));

        (text.to_owned(), state::Ast::Typings(ast))
    }

    fn parse_libraries<'a>(
        incrementor: &'a mut Incrementor,
        libraries: Vec<(internal::Library, &'a str)>,
    ) -> impl Iterator<Item = (internal::Library, Link, state::Module<()>)> + 'a {
        libraries.into_iter().map(|(library, text)| {
            let (input, ast) = Self::parse_library(&library, text);

            (
                library,
                Link::from_library(&library),
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
        F: Fn(T, &mut Context<R>) -> Internal<T2>,
    {
        let try_apply = |state, context: &mut Context<R>| -> Internal<T2> {
            context.reporter.flush()?;

            let result = f(state, context)?;

            context.reporter.flush()?;

            Ok(result)
        };

        let root_dir = self.root_dir.clone();
        self.map(|result, context| {
            let state = result.into_result()?;

            try_apply(state.clone(), context)
                .map_err(|err| Box::new(state.enrich(root_dir.clone(), *err)))
        })
    }
}

impl<S, T, R> Engine<Result<S>, R>
where
    S: Deref<Target = state::Base<T>>,
    T: Clone,
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&self) -> Writer<ast::meta::Program<T>> {
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

    fn to_writer<F, T2>(&self, f: F) -> Writer<T2>
    where
        T2: Display,
        F: Fn(&S) -> Vec<(PathBuf, T2)>,
    {
        Writer {
            files: self.state.as_ref().map(f).map_err(Clone::clone),
            verbose: self
                .state
                .as_ref()
                .map(|x| x.is_verbose())
                .unwrap_or_default(),
        }
    }
}

impl<'a> Engine<bool, FileSystem<'a>> {
    pub fn new(root_dir: &'a Path, verbose: bool) -> Self {
        let context = Context::std(Reporter::new(false), FileSystem(root_dir));

        let relative_root = if let Ok(working_dir) = current_dir() {
            root_dir.strip_prefix(working_dir).unwrap_or(root_dir)
        } else {
            root_dir
        };

        Self {
            context,
            root_dir: relative_root.to_string_lossy().to_string(),
            state: verbose,
        }
    }
}

impl<R> Engine<bool, R>
where
    R: Resolver,
{
    /// load a module tree from a single entry point
    pub fn from_entry<P>(self, entry: P) -> Engine<state::FromEntry, R>
    where
        P: AsRef<Path>,
    {
        let path = entry.as_ref();

        assert!(
            !path.is_absolute(),
            "entry must be relative to the source directory"
        );

        self.map(|verbose, _| state::FromEntry {
            entry: Link::from(path),
            verbose,
        })
    }

    /// load all modules that match a glob
    pub fn from_glob<P>(self, dir: P, glob: &str) -> Engine<Result<state::FromPaths>, R>
    where
        P: AsRef<Path>,
    {
        self.map(|verbose, _| {
            state::FromGlob {
                dir: dir.as_ref(),
                glob,
                verbose,
            }
            .to_paths()
        })
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult,
    T::Value: Clone + Enrich,
    R: Resolver,
{
    pub fn include_libraries<Library>(
        self,
        libraries: &[Library],
    ) -> Engine<Result<state::WithLibraries<T::Value, Library>>, R>
    where
        Library: internal::PlatformLibrary,
    {
        self.then(|state, _| {
            Ok(WithLibraries {
                state,
                libraries: libraries.to_vec(),
            })
        })
    }
}

impl<T, R> Engine<T, R>
where
    T: IntoResult,
    T::Value: Clone + Enrich + Traverse + ToLibraries + IsVerbose,
    R: Resolver,
{
    /// starting from the entry file recursively discover and parse modules
    pub fn parse(self) -> Engine<Result<state::Parsed>, R> {
        self.then(|state, context| {
            let mut visitor = state.traverse();
            let mut parsed = state::Parsed::new(state.is_verbose());

            for (library, link, module) in
                Self::parse_libraries(&mut parsed.incrementor().borrow_mut(), state.to_libraries())
            {
                parsed.register_library(library, link, module);
            }

            while let Some(link) = visitor.next() {
                context.load_and_parse_program(&link).map(|(text, ast)| {
                    for link in ast.to_links(&link) {
                        if !parsed.has_by_link(&link) && !link.is_library() {
                            visitor.queue(link);
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

            for (link, module) in state.libraries() {
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

                modules.keys.insert(namespace, module.id);
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
    pub fn generate<T>(&self, generator: T) -> Writer<T::Output>
    where
        T: internal::Generator<Input = ast::shape::Program>,
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
