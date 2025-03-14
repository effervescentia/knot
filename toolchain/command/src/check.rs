use crate::{log, AssertExists, Logger};
use engine::{
    engine2::{Analyzed, Builder, Context, Engine, Input, State},
    ConfigurationError,
};
use kore::{
    internal,
    pipeline::{Peek, Transform},
};
use std::path::Path;

pub struct Options<'a, Platform> {
    pub platform: Platform,

    /// absolute path to the directory containing the source code
    pub source_dir: &'a Path,

    /// path to the entry file for this application or library
    /// relative to the `source_dir`
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

fn check_plan<'a>() -> Builder<impl Transform<Context = State<'a, Logger>, In = (), Out = Analyzed>>
{
    Engine::<Logger>::plan()
        .parse()
        .peek(|(_, x)| Logger.report_parsed(x))
        .link()
        .peek(|(_, x)| Logger.report_linked(x))
        .analyze()
}

pub fn command<Platform>(opts: &Options<Platform>) -> engine::Result<()>
where
    Platform: internal::Platform<Program = lang::ast::shape::Program>,
{
    let source_dir = opts
        .source_dir
        .assert_dir_exists(ConfigurationError::SourceDirectoryNotFound)?;

    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, Platform::libraries());
    let engine = Engine::new(Context::new(source_dir, Logger));
    let plan = check_plan();

    let (_, Analyzed(ids)) = engine.execute(&plan, &input);

    log::success(opts.verbose, "analyzed", ids.len());

    Ok(())
}
