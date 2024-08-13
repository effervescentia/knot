use crate::{log, AssertExists};
use engine::{ConfigurationError, Engine};
use std::path::Path;

pub struct Options<'a> {
    /// absolute path to the directory the `glob` is evaluated within
    pub root_dir: &'a Path,

    /// pattern describing the files that should be formatted.
    /// relative to the `root_dir`
    pub glob: &'a str,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let root_dir = opts
        .root_dir
        .assert_dir_exists(ConfigurationError::RootDirectoryNotFound)?;

    log::glob(opts.glob);

    let count = Engine::new(root_dir)
        .from_glob(opts.root_dir, opts.glob)
        .parse_all()
        .inspect(|state, _| log::parsed_from_glob(state.internal_modules().count()))
        .format()
        .write(opts.root_dir)?;

    eprintln!();

    log::success("formatted", count);

    Ok(())
}
