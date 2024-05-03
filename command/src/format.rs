use crate::path::AssertExists;
use engine::{Context, Engine, FileSystem, Reporter};
use kore::color::Highlight;
use std::path::Path;

pub struct Options<'a> {
    pub root_dir: &'a Path,
    pub glob: &'a str,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let resolver = FileSystem(
        opts.root_dir
            .assert_dir_exists(engine::Error::RootDirectoryNotFound)?,
    );
    let engine = Engine::new(Context::std(Reporter::new(false), resolver));

    let count = engine
        .from_glob(opts.root_dir, opts.glob)
        .parse_matched()
        .format()
        .write(opts.root_dir)?;

    eprintln!(
        "{} {} {}\n",
        "formatted".success(),
        count.to_string().focus(),
        "module(s) with no errors".success()
    );

    Ok(())
}
