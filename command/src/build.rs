use engine::{Engine, FileSystem, Reporter};
use kore::Generator;
use lang::ast;
use std::path::Path;

pub struct Options<'a, G>
where
    G: Generator,
{
    pub generator: G,
    pub source_dir: &'a Path,
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command<G>(opts: &Options<G>) -> engine::Result<()>
where
    G: Generator<Input = ast::ProgramShape>,
{
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(Reporter::new(false), resolver);

    engine
        .from_entry(opts.entry)
        .parse_and_load()
        .link()
        .analyze()
        .generate(&opts.generator)
        .write(opts.out_dir)
}
