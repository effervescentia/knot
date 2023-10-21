use crate::link::Link;
use crate::resolve::Resolver;
use analyze::Strong;
use kore::Generator;
use lang::{
    ast::{self, ToShape},
    Program,
};
use parse::Range;
use std::{
    collections::{HashMap, VecDeque},
    fmt::Display,
    fs::File,
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

type LoadedState<T> = HashMap<Link, (String, Program<Range, T>)>;
type EntryState = Link;
type GlobState<'a> = (&'a Path, &'a str);
type ParsedState = LoadedState<()>;
type AnalyzedState = LoadedState<Strong>;

pub struct Output<T>(Vec<(PathBuf, T)>)
where
    T: Display;

impl<T> Output<T>
where
    T: Display,
{
    pub fn write(&self, dir: &Path) {
        self.0.iter().for_each(|(path, generated)| {
            let mut writer = BufWriter::new(File::create(dir.join(path)).unwrap());
            write!(writer, "{generated}").unwrap();
            writer.flush().unwrap();
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
            .map(|x| Link::from_import(&path, x))
            .collect::<Vec<_>>()
    }

    fn parse_one(resolver: &mut R, link: &Link) -> (String, Program<Range, ()>) {
        let path = link.to_path();

        let input = resolver.resolve(&path).unwrap();
        let (ast, _) = parse::parse(&input).unwrap();

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
    pub fn from_entry(self, entry: &Path) -> Engine<EntryState, R> {
        if entry.is_absolute() {
            panic!("entry must be relative to the source directory");
        }

        Engine {
            resolver: self.resolver,
            state: Link::from_path(entry),
        }
    }

    /// load all modules that match a glob
    pub fn from_glob<'a>(self, dir: &'a Path, pattern: &'a str) -> Engine<GlobState<'a>, R> {
        Engine {
            resolver: self.resolver,
            state: (dir, pattern),
        }
    }
}

impl<R> Engine<EntryState, R>
where
    R: Resolver,
{
    /// starting from the entry file recursively discover and parse modules
    pub fn parse_and_load(mut self) -> Engine<ParsedState, R> {
        let mut queue = VecDeque::from_iter(vec![self.state]);
        let mut parsed = HashMap::new();

        while !queue.is_empty() {
            let next = queue.pop_front().unwrap();

            let (input, ast) = Self::parse_one(&mut self.resolver, &next);
            let links = Self::get_links(&next, &ast);

            links.into_iter().for_each(|x| {
                if !parsed.contains_key(&x) && !queue.contains(&x) {
                    queue.push_back(x);
                }
            });

            parsed.insert(next, (input, ast));
        }

        Engine {
            resolver: self.resolver,
            state: parsed,
        }
    }
}

impl<'a, R> Engine<GlobState<'a>, R>
where
    R: Resolver,
{
    fn glob(&self) -> Vec<PathBuf> {
        let (dir, pattern) = self.state;

        glob::glob(vec![dir.to_str().unwrap(), pattern].join("/").as_str())
            .unwrap()
            .map(|x| x.unwrap())
            .map(|x| x.strip_prefix(dir).unwrap().to_path_buf())
            .collect::<Vec<_>>()
    }

    /// parse all modules that match the glob
    pub fn parse(mut self) -> Engine<ParsedState, R> {
        let mut parsed = HashMap::new();
        let links = self
            .glob()
            .into_iter()
            .map(Link::from_path)
            .collect::<Vec<_>>();

        println!("found some {links:?}");

        for link in links {
            let result = Self::parse_one(&mut self.resolver, &link);

            parsed.insert(link, result);
        }

        Engine {
            resolver: self.resolver,
            state: parsed,
        }
    }
}

impl<T, R> Engine<LoadedState<T>, R>
where
    R: Resolver,
{
    /// generate output files by formatting the loaded modules
    pub fn format(&self) -> Output<&Program<Range, T>> {
        Output(
            self.state
                .iter()
                .map(|(key, (_, ast))| (key.to_path(), ast))
                .collect::<Vec<_>>(),
        )
    }
}

impl<R> Engine<ParsedState, R>
where
    R: Resolver,
{
    pub fn analyze(self) -> Engine<AnalyzedState, R> {
        let analyzed = self
            .state
            .into_iter()
            .map(|(key, (text, ast))| {
                let typed = analyze::analyze(ast);

                (key, (text, typed))
            })
            .collect::<Vec<_>>();

        Engine {
            resolver: self.resolver,
            state: HashMap::from_iter(analyzed),
        }
    }
}

impl<R> Engine<AnalyzedState, R>
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
                .iter()
                .map(|(key, (_, ast))| generator.generate(&key.to_path(), ast.to_shape()))
                .collect(),
        )
    }
}
