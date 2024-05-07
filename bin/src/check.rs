use crate::{
    config::Config,
    path::{get_root_dir, get_source_dir, validate_entrypoint},
    reporter::{eprint_configuration, Phase},
};
use command::check;
use std::path::Path;

pub struct Args<'a> {
    pub root_dir: &'a Path,
    pub source_dir: &'a Path,
    pub entry: &'a Path,
}

impl<'a> Args<'a> {
    fn report(&self) {
        let Self {
            root_dir,
            source_dir,
            entry,
        } = self;

        eprint_configuration(vec![
            ("root_dir", Config::Path(root_dir)),
            (
                "source_dir",
                Config::rel_path(source_dir, root_dir.join(source_dir).as_path()),
            ),
            (
                "entry",
                Config::rel_path(entry, root_dir.join(source_dir).join(entry).as_path()),
            ),
        ]);
    }
}

#[allow(clippy::needless_pass_by_value)]
pub fn command(args: Args) -> engine::Result<()> {
    args.report();

    let root_dir = get_root_dir(args.root_dir)?;
    let source_dir = get_source_dir(&root_dir, args.source_dir)?;

    validate_entrypoint(&source_dir, args.entry)?;

    eprint!("{}", Phase::Execution);

    check::command(&check::Options {
        entry: args.entry,
        source_dir: &source_dir,
    })
}
