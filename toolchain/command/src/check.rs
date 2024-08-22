use crate::log;
use engine::Engine;
use std::path::Path;

pub struct Options<'a> {
    pub source_dir: &'a Path,
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    log::entrypoint(opts.verbose, opts.entry);

    let result = Engine::new(opts.source_dir, opts.verbose)
        .from_entry(opts.entry)
        .parse_and_discover()
        .inspect(|state, _| state.report_from_entry())
        .link()
        .inspect(|state, _| state.report())
        .analyze()
        .into_result()?;

    log::success(opts.verbose, "analyzed", result.1.size());

    Ok(())
}
