use crate::{engine::Engine, resolve::FileSystem};
use kore::Generator;
use lang::ast::ProgramShape;
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

pub fn command<G>(opts: &Options<G>)
where
    G: Generator<Input = ProgramShape>,
{
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(resolver);

    engine
        .parse(opts.entry)
        .analyze()
        .generate(&opts.generator)
        .write(opts.out_dir);
}
