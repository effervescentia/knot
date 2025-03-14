use crate::{log, Logger};
use engine::engine2::{Analyzed, Builder, Context, Engine, Input, State};
use kore::{
    internal,
    pipeline::{Peek, Transform},
};
use std::path::Path;

pub struct Options<'a, Platform> {
    pub platform: Platform,
    pub source_dir: &'a Path,
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

fn check_plan<'a>(
) -> Builder<impl Transform<In = (State<'a, Logger>, ()), Out = (State<'a, Logger>, Analyzed)>> {
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
    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, []);
    let engine = Engine::new(Context::new(opts.source_dir, Logger));
    let plan = check_plan();

    let (_, Analyzed(ids)) = engine.execute(&plan, &input);

    log::success(opts.verbose, "analyzed", ids.len());

    Ok(())
}
