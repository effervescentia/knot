use std::path::Path;

use crate::{engine::Engine, resolve::FileSystem};

pub struct Options<'a> {
    pub source_dir: &'a Path,
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) {
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(resolver);

    engine.parse(opts.entry).format().write(opts.out_dir);
}
