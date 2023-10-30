use engine::{Engine, FileSystem, Reporter};
use std::path::Path;

pub struct Options<'a> {
    pub source_dir: &'a Path,
    pub glob: &'a str,
}

pub fn command(opts: &Options) -> engine::Result<()> {
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(Reporter::new(false), resolver);

    engine
        .from_glob(opts.source_dir, opts.glob)
        .parse()
        .format()
        .write(opts.source_dir)
}
