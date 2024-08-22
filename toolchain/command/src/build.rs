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

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

pub fn command<G>(opts: &Options<G>) -> engine::Result<()>
where
    G: Generator<Input = ast::shape::Program>,
{
    log::entrypoint(opts.verbose, opts.entry);

    let count = Engine::new(opts.source_dir, opts.verbose)
        .from_entry(opts.entry)
        .parse_and_discover()
        .inspect(|state, _| state.report_from_entry())
        .link()
        .inspect(|state, _| state.report())
        .analyze()
        .inspect(|state, _| state.report())
        .generate(&opts.generator)
        .overwrite(opts.out_dir)?;

    log::success(opts.verbose, "transpiled", count);
    eprintln!(
        "build artifacts written to {}:\n{}\n",
        "out_dir".focus(),
        opts.out_dir.pretty()
    );

    Ok(())
}
