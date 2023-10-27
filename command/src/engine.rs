use crate::{link::Link, resolve::Resolver};
use analyze::Strong;
use kore::Generator;
use lang::{
    ast::{self, ToShape},
    Program,
};
use parse::Range;
use petgraph::{graph::Graph, Directed};
use std::{
    collections::{HashMap, VecDeque},
    fmt::Display,
    fs::{self, File},
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

mod state {
    use super::*;

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

    pub trait Modules<'a> {
        type Context;

        fn modules(&'a self) -> &'a HashMap<Link, Module<Self::Context>>;
    }

    pub struct FromEntry(pub Link);

    pub struct FromGlob<'a> {
        pub dir: &'a Path,
        pub glob: &'a str,
    }

    pub struct Parsed {
        pub modules: HashMap<Link, Module<()>>,
        pub lookup: HashMap<usize, Link>,
    }

    impl<'a> Modules<'a> for Parsed {
        type Context = ();

        fn modules(&'a self) -> &'a HashMap<Link, Module<Self::Context>> {
            &self.modules
        }
    }

    pub struct Linked {
        pub modules: HashMap<Link, Module<()>>,
        pub lookup: HashMap<usize, Link>,
        pub graph: Graph<usize, (), Directed>,
    }

    impl<'a> Modules<'a> for Linked {
        type Context = ();

        fn modules(&'a self) -> &'a HashMap<Link, Module<Self::Context>> {
            &self.modules
        }
    }

    pub struct Analyzed {
        pub modules: HashMap<Link, Module<Strong>>,
        pub lookup: HashMap<usize, Link>,
        pub graph: Graph<usize, (), Directed>,
    }

    impl<'a> Modules<'a> for Analyzed {
        type Context = Strong;

        fn modules(&'a self) -> &'a HashMap<Link, Module<Self::Context>> {
            &self.modules
        }
    }
}

pub struct Output<T>(Vec<(PathBuf, T)>)
where
    T: Display;

impl<T> Output<T>
where
    T: Display,
{
    pub fn write(&self, dir: &Path) {
        self.0.iter().for_each(|(path, generated)| {
            let path = dir.join(path);

            if let Some(parent) = path.parent() {
                fs::create_dir_all(parent).ok();
            }

            let mut writer = BufWriter::new(File::create(&path).expect(&format!(
                "failed to create file from path {}",
                path.display()
            )));
            write!(writer, "{generated}").ok();
            writer.flush().ok();
        })
    }
}

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
    fn get_links<U>(link: &Link, ast: &Program<Range, U>) -> Vec<Link> {
        let path = link.to_path();

        ast.imports()
            .iter()
            .map(|x| Link::from_import(&path, x.node().value()))
            .collect::<Vec<_>>()
    }

    fn parse_one(resolver: &mut R, link: &Link) -> (String, Program<Range, ()>) {
        let path = link.to_path();

        let input = resolver.resolve(&path).expect(&format!(
            "failed to resolve file with path {}",
            path.display()
        ));
        let (ast, _) = parse::parse(&input).expect(&format!(
            "failed to parse file with path {}",
            path.display()
        ));

        (input, ast)
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
            state: state::FromEntry(Link::from_path(entry)),
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
    pub fn parse_and_load(mut self) -> Engine<state::Parsed, R> {
        let mut next_id: usize = 0;
        let mut queue = VecDeque::from_iter(vec![self.state.0]);
        let mut parsed = HashMap::new();
        let mut lookup = HashMap::new();

        while let Some(link) = queue.pop_front() {
            let (input, ast) = Self::parse_one(&mut self.resolver, &link);
            let links = Self::get_links(&link, &ast);

            links.into_iter().for_each(|x| {
                if !parsed.contains_key(&x) && !queue.contains(&x) {
                    queue.push_back(x);
                }
            });

            lookup.insert(next_id, link.clone());
            parsed.insert(link, state::Module::new(next_id, input, ast));

            next_id += 1;
        }

        Engine {
            resolver: self.resolver,
            state: state::Parsed {
                modules: parsed,
                lookup,
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
            vec![
                dir.to_str().expect("failed to convert directory to string"),
                glob,
            ]
            .join("/")
            .as_str(),
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
    pub fn parse(mut self) -> Engine<state::Parsed, R> {
        let mut next_id: usize = 0;
        let mut parsed = HashMap::new();
        let mut lookup = HashMap::new();
        let links = self
            .glob()
            .into_iter()
            .map(Link::from_path)
            .collect::<Vec<_>>();

        println!("found some {links:?}");

        for link in links {
            let (text, ast) = Self::parse_one(&mut self.resolver, &link);

            lookup.insert(next_id, link.clone());
            parsed.insert(link, state::Module::new(next_id, text, ast));

            next_id += 1;
        }

        Engine {
            resolver: self.resolver,
            state: state::Parsed {
                modules: parsed,
                lookup,
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
    pub fn format(&'a self) -> Output<&Program<Range, S::Context>> {
        Output(
            self.state
                .modules()
                .iter()
                .map(|(link, state::Module { ast, .. })| (link.to_path(), ast))
                .collect::<Vec<_>>(),
        )
    }
}

impl<R> Engine<state::Parsed, R>
where
    R: Resolver,
{
    pub fn link(self) -> Engine<state::Linked, R> {
        let linked = self.state.modules.values().fold(Graph::new(), |acc, x| acc);

        Engine {
            resolver: self.resolver,
            state: state::Linked {
                graph: linked,
                lookup: self.state.lookup,
                modules: self.state.modules,
            },
        }
    }
}

impl<R> Engine<state::Linked, R>
where
    R: Resolver,
{
    pub fn analyze(self) -> Engine<state::Analyzed, R> {
        let analyzed = self
            .state
            .modules
            .into_iter()
            .map(|(key, state::Module { id, text, ast })| {
                let typed = analyze::analyze(ast);

                (key, state::Module::new(id, text, typed))
            })
            .collect::<Vec<_>>();

        Engine {
            resolver: self.resolver,
            state: state::Analyzed {
                graph: self.state.graph,
                lookup: self.state.lookup,
                modules: HashMap::from_iter(analyzed),
            },
        }
    }
}

impl<R> Engine<state::Analyzed, R>
where
    R: Resolver,
{
    /// generate output files using the provided generator
    pub fn generate<T>(&self, generator: &T) -> Output<T::Output>
    where
        T: Generator<Input = ast::ProgramShape>,
    {
        Output(
            self.state
                .modules
                .iter()
                .map(|(key, state::Module { ast, .. })| {
                    generator.generate(&key.to_path(), ast.to_shape())
                })
                .collect(),
        )
    }
}
