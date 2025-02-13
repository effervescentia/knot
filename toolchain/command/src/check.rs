use crate::log;
use engine::Engine;
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

    let result = Engine::new(opts.source_dir, opts.verbose)
        .from_entry(opts.entry)
        .include_libraries(&Platform::libraries())
        .parse()
        .inspect(|state, _| state.report_from_entry())
        .link()
        .inspect(|state, _| state.report())
        .analyze()
        .into_result()?;

    log::success(opts.verbose, "analyzed", result.1.size());

    Ok(())
}
