use crate::path::AssertExists;
use engine::{Context, Engine, FileSystem, Reporter};
use kore::{color::Highlight, Generator};
use lang::ast;
use std::path::Path;

pub struct Options<'a, G>
where
    G: Generator,
{
    pub generator: G,
    pub root_dir: &'a Path,
    pub source_dir: &'a Path,
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command<G>(opts: &Options<G>) -> engine::Result<()>
where
    G: Generator<Input = ast::shape::Program>,
{
    let source_path = opts
        .root_dir
        .assert_dir_exists(engine::Error::RootDirectoryNotFound)?
        .join(opts.source_dir);
    let resolver = FileSystem(
        source_path
            .as_path()
            .assert_dir_exists(engine::Error::SourceDirectoryNotFound)?,
    );
    let engine = Engine::new(Context::std(Reporter::new(false), resolver));

    source_path
        .join(opts.entry)
        .assert_file_exists(engine::Error::EntrypointNotFound)?;

    let count = engine
        .from_entry(opts.entry)
        .parse_and_discover()
        .link()
        .analyze()
        .generate(&opts.generator)
        .write(opts.out_dir)?;

    eprintln!(
        "{} {} {}\n",
        "transpiled".success(),
        count.to_string().focus(),
        "module(s) with no errors".success()
    );

    Ok(())
}
