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

type State<T> = HashMap<Link, (String, Program<Range, T>)>;
type ParsedState = State<()>;
type AnalyzedState = State<Strong>;

pub struct Generated<T>(Vec<(PathBuf, T)>)
where
    T: Display;

impl<T> Generated<T>
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

    pub fn parse(mut self, entry: &Path) -> Engine<ParsedState, R> {
        if entry.is_absolute() {
            panic!("entry must be relative to the source directory");
        }

        let mut queue = VecDeque::from_iter(vec![Link::from_path(entry)]);
        let mut parsed = HashMap::new();

        while !queue.is_empty() {
            let next = queue.pop_front().unwrap();
            let path = next.to_path();

            let input = self.resolver.resolve(&path).unwrap();
            let (ast, _) = parse::parse(&input).unwrap();

            ast.imports()
                .iter()
                .map(|x| Link::from_import(entry, x))
                .for_each(|x| {
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

impl<T, R> Engine<State<T>, R>
where
    R: Resolver,
{
    pub fn format(&self) -> Generated<&Program<Range, T>> {
        Generated(
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
    pub fn generate<T>(&self, generator: &T) -> Generated<T::Output>
    where
        T: Generator<Input = ast::ProgramShape>,
    {
        let generated = self
            .state
            .iter()
            .map(|(key, (_, ast))| generator.generate(&key.to_path(), ast.to_shape()))
            .collect::<Vec<_>>();

        Generated(generated)
    }
}
