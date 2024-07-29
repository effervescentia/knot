use crate::log;
use engine::{Context, Engine, FileSystem, Reporter};
use std::path::Path;

pub struct Options<'a> {
    pub source_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(Context::std(Reporter::new(false), resolver));

    log::entrypoint(opts.entry);

    let result = engine
        .from_entry(opts.entry)
        .parse_and_discover()
        .inspect(|state, _| log::parsed_from_entry(state.internal_modules().count()))
        .link()
        .inspect(|_, _| log::linked())
        .analyze()
        .into_result()?;

    eprintln!();

    log::success("analyzed", result.1.size());

    Ok(())
}
