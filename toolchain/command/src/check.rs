use crate::{log, Logger};
use engine::engine2::{Analyzed, Context, Engine, Input, Peek};
use kore::internal;
use std::path::Path;

pub struct Options<'a, Platform> {
    pub platform: Platform,
    pub source_dir: &'a Path,
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

pub fn command<Platform>(opts: &Options<Platform>) -> engine::Result<()>
where
    Platform: internal::Platform<Program = lang::ast::shape::Program>,
{
    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, []);
    let engine = Engine::new(Context::new(opts.source_dir, Logger));

    let plan = Engine::plan()
        .parse()
        .peek(|(_, x)| Logger.report_parsed(x))
        .link()
        .peek(|(_, x)| Logger.report_linked(x))
        .analyze();

    let (_, Analyzed(ids)) = engine.execute(&plan, &input);

    log::success(opts.verbose, "analyzed", ids.len());

    Ok(())
}
