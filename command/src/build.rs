use crate::{engine::Engine, resolve::FileSystem};

use super::TargetFormat;
use std::path::Path;

pub struct Options<'a> {
    pub target: TargetFormat,
    pub source_dir: &'a Path,
    pub out_dir: &'a Path,
    pub entry: &'a Path,
}

pub fn command(opts: &Options) {
    let resolver = FileSystem(opts.source_dir);
    let engine = Engine::new(resolver);

    engine
        .parse(opts.entry)
        .analyze()
        .generate()
        .write(opts.out_dir);
}
