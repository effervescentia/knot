use crate::resolve::Resolver;
use analyze::Strong;
use lang::{ast::ToShape, Program};
use parse::Range;
use std::{
    collections::HashMap,
    fmt::Display,
    fs::File,
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

type State<'a, T> = HashMap<&'a Path, (String, Program<Range, T>)>;
type ParsedState<'a> = State<'a, ()>;
type AnalyzedState<'a> = State<'a, Strong>;

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
        let input = self.resolver.resolve(entry).unwrap();
        let (ast, _) = parse::parse(&input).unwrap();

        Engine {
            resolver: self.resolver,
            state: HashMap::from_iter(vec![(entry, (input, ast))]),
        }
    }
}

impl<'a, T, R> Engine<State<'a, T>, R>
where
    R: Resolver,
{
    pub fn format(&self) -> Generated<&Program<Range, T>> {
        Generated(
            self.state
                .iter()
                .map(|(key, (_, ast))| (key.to_path_buf(), ast))
                .collect::<Vec<_>>(),
        )
    }
}

impl<'a, R> Engine<ParsedState<'a>, R>
where
    R: Resolver,
{
    pub fn analyze(self) -> Engine<AnalyzedState<'a>, R> {
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

impl<'a, R> Engine<AnalyzedState<'a>, R>
where
    R: Resolver,
{
    pub fn generate(&self) -> Generated<js::JavaScript> {
        let generated = self
            .state
            .iter()
            .map(|(source_path, (_, ast))| {
                let shape = ast.to_shape();
                let result = js::generate(&shape);

                (source_path.with_extension("js"), result)
            })
            .collect::<Vec<_>>();

        Generated(generated)
    }
}
