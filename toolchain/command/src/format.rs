use crate::{log, AssertExists, Logger};
use engine::{Builder, ConfigurationError, Context, Engine, Input, State};
use kore::pipeline::{Peek, Transform};
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

fn format_plan<'a, T>(
    root_dir: T,
) -> Builder<impl Transform<Context = State<'a, Logger>, In = (), Out = usize>>
where
    T: AsRef<Path>,
{
    Engine::<Logger>::plan()
        .parse()
        .peek(|(_, x)| Logger.report_parsed(x))
        .format()
        .write(root_dir)
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let root_dir = opts
        .root_dir
        .assert_dir_exists(ConfigurationError::RootDirectoryNotFound)?;

    log::glob(opts.verbose, opts.glob);

    let input: Input<_, ()> = Input::from_glob(opts.glob, []);
    let engine = Engine::new(Context::new(root_dir, Logger));
    let plan = format_plan(root_dir);

    let (_, count) = engine.execute(&plan, &input);

    log::success(opts.verbose, "formatted", count);

    Ok(())
}
