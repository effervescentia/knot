use crate::args::Target;
use command::{ast, build};
use kore::Generator;
use std::path::PathBuf;

pub struct Args {
    pub target: Target,
    pub entry: PathBuf,
    pub root_dir: PathBuf,
    pub source_dir: PathBuf,
    pub out_dir: PathBuf,
}

fn get_generator(target: Target) -> impl Generator<Input = ast::shape::Program> {
    match target {
        Target::JavaScript => js::JavaScriptGenerator::new(js::Module::ESM),
    }
}

pub fn command(
    Args {
        target,
        ref entry,
        ref root_dir,
        ref source_dir,
        ref out_dir,
    }: Args,
) -> engine::Result<()> {
    let generator = get_generator(target);

    build::command(&build::Options {
        generator,
        entry,
        root_dir,
        source_dir,
        out_dir,
    })
}
