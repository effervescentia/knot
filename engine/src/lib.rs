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
use report::ConfigurationError;
pub use report::{CodeFrame, ExecutionError, Report, Reporter};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
pub use resource::Library;
use std::{
    collections::{HashMap, HashSet, VecDeque},
    ops::Deref,
    path::Path,
};
use validate::Validator;
use write::Writer;

pub type Result<T> = std::result::Result<T, Report>;
pub type InnerResult<T> = std::result::Result<T, Vec<ExecutionError>>;

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

    fn load_and_parse_program(
        resolver: &mut R,
        link: &Link,
    ) -> InnerResult<(String, state::Ast<()>)> {
        let path = link.to_path();

        let input = resolver
            .resolve(&path)
            .ok_or(vec![ExecutionError::ModuleNotFound(link.clone())])?;

        let (ast, _) = parse::program::parse(&input)
            .map_err(|_| vec![ExecutionError::InvalidSyntax(link.clone())])?;

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

impl<'a, S, T, R> Engine<Result<S>, R>
where
    S: Deref<Target = state::Base<T>>,
    T: Clone,
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&'a self) -> Writer<&ast::meta::Program<T>> {
        Writer(
            self.state
                .as_ref()
                .map(|state| {
                    state
                        .modules()
                        .filter_map(|(link, state::Module { ast, .. })| match ast {
                            state::Ast::Program(x) if link.is_internal() => {
                                Some((link.to_path(), x))
                            }

                            state::Ast::Program(_) | state::Ast::Typings(_) => None,
                        })
                        .collect::<Vec<_>>()
                })
                .map_err(std::clone::Clone::clone),
            self.context.reporter.clone(),
        )
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
            let mut parsed = state::Parsed::default();

            for (library, link, module) in
                Self::parse_libraries(&mut incrementor, &context.libraries)
            {
                parsed.register_library(library, link, module);
            }

            while let Some(link) = queue.pop_front() {
                match Self::load_and_parse_program(&mut context.resolver, &link) {
                    Ok((input, ast)) => {
                        let links = Self::to_links(&link, &ast);

                        for link in links {
                            if !parsed.has_by_link(&link) && !queue.contains(&link) {
                                queue.push_back(link);
                            }
                        }

                        let id = NamespaceId(incrementor.increment());
                        let module = state::Module::new(id, input, ast);

                        parsed.register_source(link, module);
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(parsed)
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
                .map_err(|errs| Report::Configuration(ConfigurationError::InvalidGlob(errs)))?
                .iter()
                .map(Link::from)
                .collect::<Vec<_>>();
            let mut incrementor = Incrementor::default();
            let mut parsed = state::Parsed::default();

            for (library, link, module) in
                Self::parse_libraries(&mut incrementor, &context.libraries)
            {
                parsed.register_library(library, link, module);
            }

            for link in links {
                match Self::load_and_parse_program(&mut context.resolver, &link) {
                    Ok((text, ast)) => {
                        let id = NamespaceId(incrementor.increment());
                        let module = state::Module::new(id, text, ast);

                        parsed.register_source(link, module);
                    }

                    Err(errs) => context.reporter.raise(errs)?,
                }
            }

            context.reporter.catch()?;

            Ok(parsed)
        })
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
                        if let Some(x) = state.get_id_by_link(x) {
                            acc.add_edge(&module.id, x).ok();
                        } else {
                            context
                                .reporter
                                .report(ExecutionError::UnregisteredModule(x.clone()));
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

                let (typed, types) = module.ast.analyze(&analyze_context).map_err(|errs| {
                    context
                        .reporter
                        .finalize(Self::bind_errors(&analyze_context, errs))
                })?;

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

                let (typed, types) = module.ast.analyze(&analyze_context).map_err(|errs| {
                    context
                        .reporter
                        .finalize(Self::bind_errors(&analyze_context, errs))
                })?;

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
        Writer(
            match &self.state {
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
            },
            self.context.reporter.clone(),
        )
    }
}
