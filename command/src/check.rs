use crate::path::AssertExists;
use engine::{Context, Engine, FileSystem, Reporter};
use kore::color::Highlight;
use std::path::Path;

pub struct Options<'a> {
    pub root_dir: &'a Path,
    pub source_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) -> engine::Result<()> {
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

    let result = engine
        .from_entry(opts.entry)
        .parse_and_discover()
        .link()
        .analyze()
        .into_result()?;

    eprintln!(
        "{} {} {}\n",
        "analyzed".success(),
        result.graph.size().to_string().focus(),
        "module(s) with no errors".success()
    );

    Ok(())
}
