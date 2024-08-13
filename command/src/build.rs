use crate::log;
use engine::Engine;
use kore::{color::Highlight, pretty::Pretty, Generator};
use lang::ast;
use std::path::Path;

pub struct Options<'a, G>
where
    G: Generator,
{
    pub generator: G,
    pub out_dir: &'a Path,
    pub source_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command<G>(opts: &Options<G>) -> engine::Result<()>
where
    G: Generator<Input = ast::shape::Program>,
{
    log::entrypoint(opts.entry);

    let count = Engine::new(opts.source_dir)
        .from_entry(opts.entry)
        .parse_and_discover()
        .inspect(|state, _| log::parsed_from_entry(state.internal_modules().count()))
        .link()
        .inspect(|_, _| log::linked())
        .analyze()
        .inspect(|_, _| log::analyzed())
        .generate(&opts.generator)
        .overwrite(opts.out_dir)?;

    eprintln!();

    log::success("transpiled", count);
    eprintln!(
        "build artifacts written to {}:\n{}\n",
        "out_dir".focus(),
        opts.out_dir.pretty()
    );

    Ok(())
}
