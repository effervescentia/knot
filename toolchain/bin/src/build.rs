use crate::{
    args::Target,
    config::Config,
    log,
    path::{get_out_dir, get_root_dir, get_source_dir, validate_entrypoint},
};
use command::{build, Phase};
use kore::internal;
use lang::ast;
use std::path::Path;

pub struct Args<'a> {
    pub target: Target,
    pub entry: &'a Path,
    pub root_dir: &'a Path,
    pub source_dir: &'a Path,
    pub out_dir: &'a Path,
    pub watch: bool,
    pub verbose: bool,
}

impl Args<'_> {
    fn report(&self) {
        let Self {
            root_dir,
            source_dir,
            out_dir,
            entry,
            target,
            watch,
            verbose,
        } = self;

        if !verbose {
            return;
        }

        log::configuration(vec![
            ("root_dir", Config::Path(root_dir)),
            (
                "source_dir",
                Config::rel_path(source_dir, root_dir.join(source_dir).as_path()),
            ),
            (
                "entry",
                Config::rel_path(entry, root_dir.join(source_dir).join(entry).as_path()),
            ),
            (
                "out_dir",
                Config::rel_path(out_dir, root_dir.join(out_dir).as_path()),
            ),
            ("target", Config::Target(target)),
            ("watch", Config::Boolean(*watch)),
            ("verbose", Config::Boolean(*verbose)),
        ]);
    }
}

fn get_platform(target: Target) -> impl internal::Platform<Program = ast::shape::Program> {
    match target {
        Target::Web => web::Web,
    }
}

#[allow(clippy::needless_pass_by_value)]
pub fn command(args: Args) -> engine::Result<()> {
    args.report();

    let platform = get_platform(args.target);
    let root_dir = get_root_dir(args.root_dir)?;
    let source_dir = get_source_dir(&root_dir, args.source_dir)?;
    let out_dir = get_out_dir(&root_dir, args.out_dir);

    validate_entrypoint(&source_dir, args.entry)?;

    eprint!("{}", Phase::Execution);

    let options = build::Options {
        platform,
        entry: args.entry,
        source_dir: &source_dir,
        out_dir: &out_dir,
        verbose: args.verbose,
    };

    if args.watch {
        build::command(&options)
    } else {
        build::watch_command(&options)
    }
}
