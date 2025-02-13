use crate::{log, AssertExists, Logger};
use engine::{
    engine2::{Context, Engine, Input, Parsed, Peek},
    ConfigurationError,
};
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

    let input = Input::from_glob(opts.glob, []);
    let engine = Engine::new(Context::new(root_dir, Logger));

    let (_, count) = engine.execute(
        &Engine::plan()
            .parse()
            .peek(|(_, Parsed(ids))| {
                Logger.report_parsed(ids.len());
            })
            .format()
            .write(),
        &input,
    );

    log::success(opts.verbose, "formatted", count);

    Ok(())
}
