use crate::{log, AssertExists};
use engine::{ConfigurationError, Engine};
use std::path::Path;

pub struct Options<'a> {
    /// absolute path to the directory the `glob` is evaluated within
    pub root_dir: &'a Path,

    /// pattern describing the files that should be formatted.
    /// relative to the `root_dir`
    pub glob: &'a str,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let root_dir = opts
        .root_dir
        .assert_dir_exists(ConfigurationError::RootDirectoryNotFound)?;

    log::glob(opts.verbose, opts.glob);

    let count = Engine::new(root_dir, opts.verbose)
        .from_glob(opts.root_dir, opts.glob)
        .parse_all()
        .inspect(|state, _| state.report_from_glob())
        .format()
        .write(opts.root_dir)?;

    log::success(opts.verbose, "formatted", count);

    Ok(())
}
